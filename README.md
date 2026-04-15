# Analisador Lexico

Projeto em C para analise lexica de arquivos `.pas` (MicroPascal). O programa le um arquivo fonte e gera, no mesmo diretorio, os arquivos de saida com tokens, tabela de simbolos e erros lexicos.

## Requisitos

- `gcc`
- `make`

## Como compilar

Na raiz do projeto, rode:

```bash
make
```

Isso compila o projeto com `gcc -std=c99` e gera o executavel `./lexico`.

## Como executar

Passe o caminho de um arquivo `.pas` como argumento:

```bash
./lexico caminho/arquivo.pas
```

Exemplo com um arquivo de teste do repositorio:

```bash
./lexico testes/teste1.pas
```

## Arquivos gerados

Ao executar o programa, ele cria os arquivos abaixo ao lado do `.pas` informado:

- `arquivo.lex`: lista de tokens encontrados
- `arquivo.ts`: tabela de simbolos
- `arquivo.err`: erros lexicos encontrados

Exemplo:

```bash
./lexico testes/teste1.pas
```

Gera:

- `testes/teste1.lex`
- `testes/teste1.ts`
- `testes/teste1.err`

## Teste rapido

Para compilar e rodar um caso de exemplo:

```bash
make
./lexico testes/teste1.pas
```

## Limpeza

Para remover o executavel e arquivos temporarios de compilacao:

```bash
make clean
```

## Estrutura basica

- `main.c`: ponto de entrada e geracao dos arquivos de saida
- `lexico.c`: implementacao do analisador lexico
- `lexico.h`: definicoes compartilhadas
- `testes/`: arquivos `.pas` e saidas esperadas
