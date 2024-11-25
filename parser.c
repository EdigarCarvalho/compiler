#include "parser.h"
#include <stdlib.h>
#include <string.h>

void setError(const char* message, int line, int column, const char* context) {
    currentError.message = message;
    currentError.line = line;
    currentError.column = column;
    strncpy(currentError.context, context, MAX_ERROR_LENGTH - 1);
}

const char* getErrorMessage() {
    return currentError.message;
}

void clearError() {
    currentError.message = NULL;
    currentError.line = 0;
    currentError.column = 0;
    currentError.context[0] = '\0';
}

TokenBuffer* createTokenBuffer() {
    TokenBuffer* buffer = malloc(sizeof(TokenBuffer));
    buffer->capacity = INITIAL_TOKEN_BUFFER_SIZE;
    buffer->tokens = malloc(sizeof(Token) * buffer->capacity);
    buffer->count = 0;
    return buffer;
}

void addTokenToBuffer(TokenBuffer* buffer, Token token) {
    if (buffer->count >= buffer->capacity) {
        buffer->capacity *= 2;
        buffer->tokens = realloc(buffer->tokens, sizeof(Token) * buffer->capacity);
    }
    buffer->tokens[buffer->count++] = token;
}

void freeTokenBuffer(TokenBuffer* buffer) {
    free(buffer->tokens);
    free(buffer);
}

bool parseColumnList(TokenBuffer* buffer, int* current) {
    do {
        if (buffer->tokens[*current].type != TOKEN_IDENTIFIER) {
            setError("Expected column name", 
                    buffer->tokens[*current].line,
                    buffer->tokens[*current].column,
                    buffer->tokens[*current].value);
            return false;
        }
        (*current)++;

        // Verificar se há mais tokens
        if (*current >= buffer->count) {
            setError("Unexpected end of input after column name",
                    buffer->tokens[*current - 1].line,
                    buffer->tokens[*current - 1].column,
                    buffer->tokens[*current - 1].value);
            return false;
        }

        // Se o próximo token for uma vírgula, avançar e continuar o loop
        if (buffer->tokens[*current].type == TOKEN_DELIMITER &&
            strcmp(buffer->tokens[*current].value, ",") == 0) {
            (*current)++;
            // Verificar se há mais tokens após a vírgula
            if (*current >= buffer->count) {
                setError("Unexpected end of input after comma",
                        buffer->tokens[*current - 1].line,
                        buffer->tokens[*current - 1].column,
                        buffer->tokens[*current - 1].value);
                return false;
            }
        } else {
            break; // Se não for vírgula, sair do loop
        }
    } while (true);

    return true;
}

bool isSelectStatement(TokenBuffer* buffer, int* current) {
    // Verificar se é SELECT
    if (buffer->tokens[*current].type != TOKEN_KEYWORD ||
        strcmp(buffer->tokens[*current].value, "SELECT") != 0) {
        return false;
    }

    (*current)++;

    // Verificar se há mais tokens após SELECT
    if (*current >= buffer->count) {
        setError("Unexpected end of input after SELECT",
                buffer->tokens[*current - 1].line,
                buffer->tokens[*current - 1].column,
                buffer->tokens[*current - 1].value);
        return false;
    }

    // Processar lista de colunas
    if (!parseColumnList(buffer, current)) {
        return false;
    }

    // Verificar FROM
    if (buffer->tokens[*current].type != TOKEN_KEYWORD ||
        strcmp(buffer->tokens[*current].value, "FROM") != 0) {
        setError("Expected FROM keyword",
                buffer->tokens[*current].line,
                buffer->tokens[*current].column,
                buffer->tokens[*current].value);
        return false;
    }
    (*current)++;

    // Verificar se há mais tokens após FROM
    if (*current >= buffer->count) {
        setError("Unexpected end of input after FROM",
                buffer->tokens[*current - 1].line,
                buffer->tokens[*current - 1].column,
                buffer->tokens[*current - 1].value);
        return false;
    }

    // Verificar nome da tabela
    if (buffer->tokens[*current].type != TOKEN_IDENTIFIER) {
        setError("Expected table name",
                buffer->tokens[*current].line,
                buffer->tokens[*current].column,
                buffer->tokens[*current].value);
        return false;
    }
    (*current)++;

    // Verificar se há mais tokens
    if (*current >= buffer->count) {
        setError("Unexpected end of input after table name",
                buffer->tokens[*current - 1].line,
                buffer->tokens[*current - 1].column,
                buffer->tokens[*current - 1].value);
        return false;
    }

    // Verificar cláusula WHERE opcional
    if (*current < buffer->count && 
        buffer->tokens[*current].type == TOKEN_KEYWORD &&
        strcmp(buffer->tokens[*current].value, "WHERE") == 0) {
        (*current)++;
        
        if (!parseWhereClause(buffer, current)) {
            return false;
        }
    }

    // Verificar ponto e vírgula
    if (*current >= buffer->count || buffer->tokens[*current].type != TOKEN_SEMICOLON) {
        setError("Expected semicolon",
                buffer->tokens[*current - 1].line,
                buffer->tokens[*current - 1].column,
                buffer->tokens[*current - 1].value);
        return false;
    }
    (*current)++;

    return true;
}

bool parseWhereClause(TokenBuffer* buffer, int* current) {
    // Verificar se ainda há tokens para processar
    if (*current >= buffer->count) {
        setError("Unexpected end of input in WHERE clause",
                buffer->tokens[*current - 1].line,
                buffer->tokens[*current - 1].column,
                buffer->tokens[*current - 1].value);
        return false;
    }

    // Coluna
    if (buffer->tokens[*current].type != TOKEN_IDENTIFIER) {
        setError("Expected column name in WHERE clause",
                buffer->tokens[*current].line,
                buffer->tokens[*current].column,
                buffer->tokens[*current].value);
        return false;
    }
    (*current)++;

    // Verificar se há mais tokens
    if (*current >= buffer->count) {
        setError("Unexpected end of input after column name in WHERE clause",
                buffer->tokens[*current - 1].line,
                buffer->tokens[*current - 1].column,
                buffer->tokens[*current - 1].value);
        return false;
    }

    // Operador
    if (buffer->tokens[*current].type != TOKEN_OPERATOR) {
        setError("Expected comparison operator",
                buffer->tokens[*current].line,
                buffer->tokens[*current].column,
                buffer->tokens[*current].value);
        return false;
    }
    (*current)++;

    // Verificar se há mais tokens
    if (*current >= buffer->count) {
        setError("Unexpected end of input after operator in WHERE clause",
                buffer->tokens[*current - 1].line,
                buffer->tokens[*current - 1].column,
                buffer->tokens[*current - 1].value);
        return false;
    }

    // Valor
    TokenType type = buffer->tokens[*current].type;
    if (type != TOKEN_INTEGER && type != TOKEN_FLOAT && 
        type != TOKEN_STRING && type != TOKEN_IDENTIFIER) {
        setError("Expected value in WHERE clause",
                buffer->tokens[*current].line,
                buffer->tokens[*current].column,
                buffer->tokens[*current].value);
        return false;
    }
    (*current)++;

    return true;
}

void parseTokenBuffer(TokenBuffer* buffer) {
    int current = 0;

    while (current < buffer->count) {
        Token token = buffer->tokens[current];
        
        if (token.type == TOKEN_KEYWORD) {
            if (strcmp(token.value, "SELECT") == 0) {
                if (!isSelectStatement(buffer, &current)) {
                    return;
                }
            } else {
                setError("Unsupported SQL statement",
                        token.line, token.column, token.value);
                return;
            }
        } else if (token.type != TOKEN_EOF) {
            setError("Expected SQL statement",
                    token.line, token.column, token.value);
            return;
        }
    }
}

bool performSemanticAnalysis(TokenBuffer* buffer) {
    SemanticContext semanticContext;
    initSemanticContext(&semanticContext);

    // Populate semantic context from token buffer
    // This is a simplified example, you'll need to enhance this
    for (int i = 0; i < buffer->count; i++) {
        Token token = buffer->tokens[i];
        
        // Example: Add tables and columns
        if (token.type == TOKEN_KEYWORD && strcmp(token.value, "FROM") == 0 && i+1 < buffer->count) {
            // Assuming next token is table name
            Table* table = malloc(sizeof(Table));
            strcpy(table->name, buffer->tokens[i+1].value);
            table->columnCount = 0;  // You'll populate this from symbol table
            
            addTable(&semanticContext, table);
        }
    }
    

    bool result = analyzeSemanticRules(&semanticContext);

    // Print errors if any
    if (!result) {
        printf("Semantic Analysis Errors:\n");
        for (int i = 0; i < semanticContext.errorCount; i++) {
            printf("- %s\n", semanticContext.errors[i]);
        }
    }

    freeSemanticContext(&semanticContext);
    return result;
}

void addTable(SemanticContext* context, Table* table) {
    if (context->tableCount < MAX_TABLES) {
        context->tables[context->tableCount++] = table;
    }
}