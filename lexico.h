#ifndef LEXICO_H
#define LEXICO_H

#include <stdio.h>

#define MAX_LEXEMA 100
#define MAX_TS     500

typedef enum {
    KW_PROGRAM, KW_VAR, KW_INTEGER, KW_REAL,
    KW_BEGIN, KW_END, KW_IF, KW_THEN, KW_ELSE,
    KW_WHILE, KW_DO,
    ID,
    NUM_INT, NUM_REAL,
    OP_EQ, OP_NE, OP_LT, OP_LE, OP_GT, OP_GE,
    OP_AD, OP_MIN, OP_MUL, OP_DIV, OP_ASS,
    SMB_SEM, SMB_COM, SMB_OPA, SMB_CPA,
    SMB_COL, SMB_DOT,
    TOKEN_EOF, TOKEN_ERR
} TipoToken;

typedef enum {
    ERRO_NENHUM,
    ERRO_CARACTERE_INVALIDO,
    ERRO_COMENTARIO_NAO_FECHADO,
    ERRO_STRING_NAO_FECHADA,
    ERRO_STRING_NAO_SUPORTADA,
    ERRO_NUMERO_MALFORMADO
} TipoErroLexico;

typedef struct {
    TipoToken tipo;
    TipoErroLexico erro;
    char lexema[MAX_LEXEMA];
    int linha;
    int coluna;
} Token;

typedef struct {
    char lexema[MAX_LEXEMA];
    TipoToken tipo;
} EntradaTS;

extern EntradaTS TS[MAX_TS];
extern int numTS;

void        inicializaAnalisador(FILE *arq);
Token       proximoToken(void);
const char *nomeToken(TipoToken t);

int  buscarTS(const char *lex);
void inserirTS(const char *lex, TipoToken t);

#endif
