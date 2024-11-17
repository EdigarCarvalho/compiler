#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"

void compileSQL(const char* filename) {
    FILE *input = fopen(filename, "r");
    if (!input) {
        printf("Erro: Não foi possível abrir o arquivo '%s'\n", filename);
        return;
    }

    printf("Iniciando compilação SQL do arquivo: %s\n\n", filename);
    
    // Primeira passagem: Análise Léxica
    printf("=== Análise Léxica ===\n");
    int line = 1, column = 1;
    Token token;
    int lexicalErrors = 0;

    // Criar buffer para tokens
    TokenBuffer* tokenBuffer = createTokenBuffer();

    do {
        token = getNextToken(input, &line, &column);
        
        // Armazenar token no buffer para análise sintática
        if (token.type != TOKEN_ERROR && token.type != TOKEN_COMMENT) {
            addTokenToBuffer(tokenBuffer, token);
        }
        
        // Imprimir informação do token
        printf("Token: { Tipo: %s, Valor: '%s', Linha: %d, Coluna: %d }\n",
               TokenTypeNames[token.type], token.value, token.line, token.column);
               
        if (token.type == TOKEN_ERROR) {
            printf("\nErro Léxico na linha %d, coluna %d: %s\n",
                   token.line, token.column, token.value);
            lexicalErrors++;
        }
    } while (token.type != TOKEN_EOF && lexicalErrors < 10);

    if (lexicalErrors > 0) {
        printf("\nCompilação interrompida devido a erros léxicos\n");
        freeTokenBuffer(tokenBuffer);
        fclose(input);
        return;
    }

    // Imprimir tabela de símbolos
    printf("\nTabela de Símbolos após Análise Léxica:\n");
    printSymbolTable();
    
    printf("\n=== Análise Sintática ===\n");
    // Passar o buffer de tokens para o parser ao invés do arquivo
    parseTokenBuffer(tokenBuffer);

    // Verificar erros sintáticos
    if (getErrorMessage() != NULL) {
        printf("\nErro Sintático na linha %d, coluna %d: %s\n",
               currentError.line, currentError.column, getErrorMessage());
    } else {
        printf("\nAnálise sintática completada com sucesso\n");
        
        // Adicionar análise semântica básica
        printf("\n=== Análise Semântica ===\n");
        if (performSemanticAnalysis(tokenBuffer)) {
            printf("Análise semântica completada com sucesso\n");
        }
    }

    freeTokenBuffer(tokenBuffer);
    fclose(input);
    printf("\nCompilação finalizada.\n");
}

int main(int argc, char *argv[]) {
    const char* filename = (argc > 1) ? argv[1] : "test.sql";
    compileSQL(filename);
    return 0;
}

// gcc compiler.c lexico.c parser.c -o sqlcompiler
// ./sqlcompiler test.sql