#include "lexico.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    S_INICIAL = 0,
    S_ID,
    S_INT,
    S_REAL,
    S_REAL_E,
    S_REAL_ES,
    S_REAL_ED,
    S_INT_E,
    S_INT_ES,
    S_INT_ED,
    S_NUM_MAL,
    S_MENOR,
    S_MAIOR,
    S_DPONTOS,
    S_COMMENT,
    S_STRING
};

static FILE *fonte = NULL;
static int linha_atual = 1;
static int coluna_atual = 0;

#define PB_MAX 3
static int pb_buf[PB_MAX];
static int pb_cnt = 0;

EntradaTS TS[MAX_TS];
int numTS = 0;

static const char *kw_nomes[] = { "program", "var", "integer", "real", "begin", "end", "if", "then", "else", "while", "do", NULL };

static TipoToken kw_tipos[] = { KW_PROGRAM, KW_VAR, KW_INTEGER, KW_REAL, KW_BEGIN, KW_END, KW_IF, KW_THEN, KW_ELSE, KW_WHILE, KW_DO };

static void copiarLexema(char *destino, const char *origem) {
    strncpy(destino, origem, MAX_LEXEMA - 1);
    destino[MAX_LEXEMA - 1] = '\0';
}

static void adicionarChar(char *buf, int *pos, int c) {
    if (*pos < MAX_LEXEMA - 1) {
        buf[*pos] = (char)c;
        (*pos)++;
        buf[*pos] = '\0';
    }
}

static Token criarToken(TipoToken tipo, const char *lexema, int linha, int coluna) {
    Token tok;

    tok.tipo = tipo;
    tok.erro = ERRO_NENHUM;
    tok.linha = linha;
    tok.coluna = coluna;
    copiarLexema(tok.lexema, lexema);
    return tok;
}

static Token criarErro(TipoErroLexico erro, const char *lexema, int linha, int coluna) {
    Token tok;

    tok.tipo = TOKEN_ERR;
    tok.erro = erro;
    tok.linha = linha;
    tok.coluna = coluna;
    copiarLexema(tok.lexema, lexema);
    return tok;
}

static int ehSeparadorNumeroMalformado(int c) {
    if (c == EOF) {
        return 1;
    }
    if (isspace((unsigned char)c)) {
        return 1;
    }

    switch (c) {
        case '=':
        case '<':
        case '>':
        case ':':
        case '+':
        case '-':
        case '*':
        case '/':
        case ';':
        case ',':
        case '(':
        case ')':
        case '.':
        case '{':
        case '}':
        case '\'':
            return 1;
        default:
            return 0;
    }
}

int buscarTS(const char *lex) {
    int i;

    for (i = 0; i < numTS; i++) {
        if (strcmp(TS[i].lexema, lex) == 0) {
            return i;
        }
    }
    return -1;
}

void inserirTS(const char *lex, TipoToken t) {
    if (buscarTS(lex) == -1 && numTS < MAX_TS) {
        copiarLexema(TS[numTS].lexema, lex);
        TS[numTS].tipo = t;
        numTS++;
    }
}

static int lerChar(void) {
    int c;

    if (pb_cnt > 0) {
        c = pb_buf[--pb_cnt];
    } else {
        c = fgetc(fonte);
    }

    if (c == '\n') {
        linha_atual++;
        coluna_atual = 0;
    } else if (c != EOF) {
        coluna_atual++;
    }

    return c;
}

static void devolverChar(int c) {
    if (c == EOF) {
        return;
    }

    if (pb_cnt < PB_MAX) {
        pb_buf[pb_cnt++] = c;
        if (c == '\n') {
            linha_atual--;
        } else {
            coluna_atual--;
        }
    }
}

void inicializaAnalisador(FILE *arq) {
    int i;

    fonte = arq;
    linha_atual = 1;
    coluna_atual = 0;
    pb_cnt = 0;
    numTS = 0;

    for (i = 0; kw_nomes[i] != NULL; i++) {
        inserirTS(kw_nomes[i], kw_tipos[i]);
    }
}

const char *nomeToken(TipoToken t) {
    switch (t) {
        case KW_PROGRAM: return "KW_PROGRAM";
        case KW_VAR: return "KW_VAR";
        case KW_INTEGER: return "KW_INTEGER";
        case KW_REAL: return "KW_REAL";
        case KW_BEGIN: return "KW_BEGIN";
        case KW_END: return "KW_END";
        case KW_IF: return "KW_IF";
        case KW_THEN: return "KW_THEN";
        case KW_ELSE: return "KW_ELSE";
        case KW_WHILE: return "KW_WHILE";
        case KW_DO: return "KW_DO";
        case ID: return "ID";
        case NUM_INT: return "NUM_INT";
        case NUM_REAL: return "NUM_REAL";
        case OP_EQ: return "OP_EQ";
        case OP_NE: return "OP_NE";
        case OP_LT: return "OP_LT";
        case OP_LE: return "OP_LE";
        case OP_GT: return "OP_GT";
        case OP_GE: return "OP_GE";
        case OP_AD: return "OP_AD";
        case OP_MIN: return "OP_MIN";
        case OP_MUL: return "OP_MUL";
        case OP_DIV: return "OP_DIV";
        case OP_ASS: return "OP_ASS";
        case SMB_SEM: return "SMB_SEM";
        case SMB_COM: return "SMB_COM";
        case SMB_OPA: return "SMB_OPA";
        case SMB_CPA: return "SMB_CPA";
        case SMB_COL: return "SMB_COL";
        case SMB_DOT: return "SMB_DOT";
        case TOKEN_EOF: return "EOF";
        default: return "ERR";
    }
}

Token proximoToken(void) {
    int estado = S_INICIAL;
    int pos = 0;
    int c;
    int idx;
    int lin_tok = 1;
    int col_tok = 1;
    int lin_coment = 0;
    int col_coment = 0;
    char buf[MAX_LEXEMA];

    memset(buf, 0, sizeof(buf));

    while (1) {
        c = lerChar();

        switch (estado) {
            case S_INICIAL:
                if (c == EOF) {
                    return criarToken(TOKEN_EOF, "EOF", linha_atual, coluna_atual);
                }

                if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                    break;
                }

                lin_tok = linha_atual;
                col_tok = coluna_atual;
                pos = 0;
                buf[0] = '\0';

                if (isalpha((unsigned char)c)) {
                    adicionarChar(buf, &pos, tolower((unsigned char)c));
                    estado = S_ID;
                } else if (isdigit((unsigned char)c)) {
                    adicionarChar(buf, &pos, c);
                    estado = S_INT;
                } else if (c == '<') {
                    estado = S_MENOR;
                } else if (c == '>') {
                    estado = S_MAIOR;
                } else if (c == ':') {
                    estado = S_DPONTOS;
                } else if (c == '{') {
                    lin_coment = linha_atual;
                    col_coment = coluna_atual;
                    estado = S_COMMENT;
                } else if (c == '\'') {
                    adicionarChar(buf, &pos, c);
                    estado = S_STRING;
                } else {
                    switch (c) {
                        case '=':
                            return criarToken(OP_EQ, "=", lin_tok, col_tok);
                        case '+':
                            return criarToken(OP_AD, "+", lin_tok, col_tok);
                        case '-':
                            return criarToken(OP_MIN, "-", lin_tok, col_tok);
                        case '*':
                            return criarToken(OP_MUL, "*", lin_tok, col_tok);
                        case '/':
                            return criarToken(OP_DIV, "/", lin_tok, col_tok);
                        case ';':
                            return criarToken(SMB_SEM, ";", lin_tok, col_tok);
                        case ',':
                            return criarToken(SMB_COM, ",", lin_tok, col_tok);
                        case '(':
                            return criarToken(SMB_OPA, "(", lin_tok, col_tok);
                        case ')':
                            return criarToken(SMB_CPA, ")", lin_tok, col_tok);
                        case '.':
                            return criarToken(SMB_DOT, ".", lin_tok, col_tok);
                        default:
                            adicionarChar(buf, &pos, c);
                            return criarErro(ERRO_CARACTERE_INVALIDO, buf, lin_tok, col_tok);
                    }
                }
                break;

            case S_ID:
                if (isalpha((unsigned char)c) || isdigit((unsigned char)c)) {
                    adicionarChar(buf, &pos, tolower((unsigned char)c));
                } else {
                    devolverChar(c);
                    idx = buscarTS(buf);
                    if (idx != -1 && TS[idx].tipo != ID) {
                        return criarToken(TS[idx].tipo, buf, lin_tok, col_tok);
                    }

                    inserirTS(buf, ID);
                    return criarToken(ID, buf, lin_tok, col_tok);
                }
                break;

            case S_INT:
                if (isdigit((unsigned char)c)) {
                    adicionarChar(buf, &pos, c);
                } else if (c == '.') {
                    int prox = lerChar();

                    if (isdigit((unsigned char)prox)) {
                        adicionarChar(buf, &pos, '.');
                        adicionarChar(buf, &pos, prox);
                        estado = S_REAL;
                    } else {
                        devolverChar(prox);
                        devolverChar('.');
                        return criarToken(NUM_INT, buf, lin_tok, col_tok);
                    }
                } else if (c == 'E' || c == 'e') {
                    adicionarChar(buf, &pos, c);
                    estado = S_INT_E;
                } else {
                    devolverChar(c);
                    return criarToken(NUM_INT, buf, lin_tok, col_tok);
                }
                break;

            case S_REAL:
                if (isdigit((unsigned char)c)) {
                    adicionarChar(buf, &pos, c);
                } else if (c == 'E' || c == 'e') {
                    adicionarChar(buf, &pos, c);
                    estado = S_REAL_E;
                } else {
                    devolverChar(c);
                    return criarToken(NUM_REAL, buf, lin_tok, col_tok);
                }
                break;

            case S_REAL_E:
                if (c == '+' || c == '-') {
                    adicionarChar(buf, &pos, c);
                    estado = S_REAL_ES;
                } else if (isdigit((unsigned char)c)) {
                    adicionarChar(buf, &pos, c);
                    estado = S_REAL_ED;
                } else if (isalpha((unsigned char)c)) {
                    adicionarChar(buf, &pos, c);
                    estado = S_NUM_MAL;
                } else {
                    devolverChar(c);
                    return criarErro(ERRO_NUMERO_MALFORMADO, buf, lin_tok, col_tok);
                }
                break;

            case S_REAL_ES:
                if (isdigit((unsigned char)c)) {
                    adicionarChar(buf, &pos, c);
                    estado = S_REAL_ED;
                } else if (isalpha((unsigned char)c)) {
                    adicionarChar(buf, &pos, c);
                    estado = S_NUM_MAL;
                } else {
                    devolverChar(c);
                    return criarErro(ERRO_NUMERO_MALFORMADO, buf, lin_tok, col_tok);
                }
                break;

            case S_REAL_ED:
                if (isdigit((unsigned char)c)) {
                    adicionarChar(buf, &pos, c);
                } else if (isalpha((unsigned char)c)) {
                    adicionarChar(buf, &pos, c);
                    estado = S_NUM_MAL;
                } else {
                    devolverChar(c);
                    return criarToken(NUM_REAL, buf, lin_tok, col_tok);
                }
                break;

            case S_INT_E:
                if (c == '+' || c == '-') {
                    adicionarChar(buf, &pos, c);
                    estado = S_INT_ES;
                } else if (isdigit((unsigned char)c)) {
                    adicionarChar(buf, &pos, c);
                    estado = S_INT_ED;
                } else if (isalpha((unsigned char)c)) {
                    adicionarChar(buf, &pos, c);
                    estado = S_NUM_MAL;
                } else {
                    devolverChar(c);
                    return criarErro(ERRO_NUMERO_MALFORMADO, buf, lin_tok, col_tok);
                }
                break;

            case S_INT_ES:
                if (isdigit((unsigned char)c)) {
                    adicionarChar(buf, &pos, c);
                    estado = S_INT_ED;
                } else if (isalpha((unsigned char)c)) {
                    adicionarChar(buf, &pos, c);
                    estado = S_NUM_MAL;
                } else {
                    devolverChar(c);
                    return criarErro(ERRO_NUMERO_MALFORMADO, buf, lin_tok, col_tok);
                }
                break;

            case S_INT_ED:
                if (isdigit((unsigned char)c)) {
                    adicionarChar(buf, &pos, c);
                } else if (isalpha((unsigned char)c)) {
                    adicionarChar(buf, &pos, c);
                    estado = S_NUM_MAL;
                } else {
                    devolverChar(c);
                    return criarErro(ERRO_NUMERO_MALFORMADO, buf, lin_tok, col_tok);
                }
                break;

            case S_NUM_MAL:
                if (ehSeparadorNumeroMalformado(c)) {
                    devolverChar(c);
                    return criarErro(ERRO_NUMERO_MALFORMADO, buf, lin_tok, col_tok);
                }

                adicionarChar(buf, &pos, c);
                break;

            case S_MENOR:
                if (c == '=') {
                    return criarToken(OP_LE, "<=", lin_tok, col_tok);
                }
                if (c == '>') {
                    return criarToken(OP_NE, "<>", lin_tok, col_tok);
                }
                devolverChar(c);
                return criarToken(OP_LT, "<", lin_tok, col_tok);

            case S_MAIOR:
                if (c == '=') {
                    return criarToken(OP_GE, ">=", lin_tok, col_tok);
                }
                devolverChar(c);
                return criarToken(OP_GT, ">", lin_tok, col_tok);

            case S_DPONTOS:
                if (c == '=') {
                    return criarToken(OP_ASS, ":=", lin_tok, col_tok);
                }
                devolverChar(c);
                return criarToken(SMB_COL, ":", lin_tok, col_tok);

            case S_COMMENT:
                if (c == EOF) {
                    return criarErro(ERRO_COMENTARIO_NAO_FECHADO, "", lin_coment, col_coment);
                }
                if (c == '}') {
                    estado = S_INICIAL;
                }
                break;

            case S_STRING:
                if (c == EOF || c == '\n') {
                    return criarErro(ERRO_STRING_NAO_FECHADA, buf, lin_tok, col_tok);
                }

                adicionarChar(buf, &pos, c);
                if (c == '\'') {
                    return criarErro(ERRO_STRING_NAO_SUPORTADA, buf, lin_tok, col_tok);
                }
                break;
        }
    }
}
