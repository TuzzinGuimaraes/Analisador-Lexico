#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexico.h"

static void escreveErroLexico(FILE *f_err, const Token *tok) {
    switch (tok->erro) {
        case ERRO_COMENTARIO_NAO_FECHADO:
            fprintf(f_err,
                    "ERRO lexico: comentario nao fechado na linha %d, coluna %d\n",
                    tok->linha, tok->coluna);
            break;
        case ERRO_STRING_NAO_FECHADA:
            fprintf(f_err,
                    "ERRO lexico: string nao fechada na linha %d, coluna %d\n",
                    tok->linha, tok->coluna);
            break;
        case ERRO_STRING_NAO_SUPORTADA:
            fprintf(f_err,
                    "ERRO lexico: string nao suportada na linha %d, coluna %d\n",
                    tok->linha, tok->coluna);
            break;
        case ERRO_NUMERO_MALFORMADO:
            fprintf(f_err,
                    "ERRO lexico: numero malformado '%s' na linha %d, coluna %d\n",
                    tok->lexema, tok->linha, tok->coluna);
            break;
        case ERRO_CARACTERE_INVALIDO:
        default:
            fprintf(f_err,
                    "ERRO lexico: caractere invalido '%s' na linha %d, coluna %d\n",
                    tok->lexema, tok->linha, tok->coluna);
            break;
    }
}

int main(int argc, char *argv[]) {
    FILE *f_fonte, *f_lex, *f_ts, *f_err;
    char base[256];
    char arq_lex[264], arq_ts[264], arq_err[264];
    Token tok;
    int num_erros = 0;
    int i;
    char *ponto;

    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo.pas>\n", argv[0]);
        return 1;
    }

    strncpy(base, argv[1], sizeof(base) - 1);
    base[sizeof(base) - 1] = '\0';
    ponto = strrchr(base, '.');
    if (ponto) *ponto = '\0';

    snprintf(arq_lex, sizeof(arq_lex), "%s.lex", base);
    snprintf(arq_ts,  sizeof(arq_ts),  "%s.ts",  base);
    snprintf(arq_err, sizeof(arq_err), "%s.err",  base);

    f_fonte = fopen(argv[1], "r");
    if (!f_fonte) {
        perror("Erro ao abrir arquivo fonte");
        return 1;
    }

    f_lex = fopen(arq_lex, "w");
    f_ts  = fopen(arq_ts,  "w");
    f_err = fopen(arq_err, "w");
    if (!f_lex || !f_ts || !f_err) {
        perror("Erro ao criar arquivos de saida");
        fclose(f_fonte);
        return 1;
    }

    inicializaAnalisador(f_fonte);

    while (1) {
        tok = proximoToken();

        if (tok.tipo == TOKEN_EOF)
            break;

        if (tok.tipo == TOKEN_ERR) {
            num_erros++;
            escreveErroLexico(f_err, &tok);
            continue;
        }

        fprintf(f_lex, "<%s, %s> %d %d\n",
                nomeToken(tok.tipo), tok.lexema, tok.linha, tok.coluna);
    }

    fprintf(f_ts, "%-30s %s\n", "Lexema", "Tipo");
    fprintf(f_ts, "%-30s %s\n",
            "------------------------------", "--------------------");
    for (i = 0; i < numTS; i++) {
        fprintf(f_ts, "%-30s %s\n", TS[i].lexema, nomeToken(TS[i].tipo));
    }

    if (num_erros == 0) {
        fprintf(f_err, "Nenhum erro lexico encontrado.\n");
    }

    printf("Analise lexica concluida.\n");
    printf("  Tokens:           %s\n", arq_lex);
    printf("  Tabela simbolos:  %s\n", arq_ts);
    printf("  Erros:            %s\n", arq_err);
    if (num_erros > 0)
        printf("  Total de erros:   %d\n", num_erros);

    fclose(f_fonte);
    fclose(f_lex);
    fclose(f_ts);
    fclose(f_err);

    return 0;
}
