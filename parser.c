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

bool parseSelectStatement(TokenBuffer* buffer, int* current) {
    // Verify SELECT keyword
    if (buffer->tokens[*current].type != TOKEN_KEYWORD ||
        strcmp(buffer->tokens[*current].value, "SELECT") != 0) {
        return false;
    }
    (*current)++;

    // Enhanced projection list parsing (supporting functions and aliases)
    while (*current < buffer->count && 
           buffer->tokens[*current].type != TOKEN_KEYWORD &&
           strcmp(buffer->tokens[*current].value, "FROM") != 0) {
        
        // Support for function calls like COUNT(), SUM(), MAX()
        if (buffer->tokens[*current].type == TOKEN_KEYWORD) {
            // Check if it's an aggregate function
            const char* functions[] = {"COUNT", "SUM", "MAX", "MIN", "AVG"};
            bool isFunction = false;
            size_t functionCount = sizeof(functions)/sizeof(functions[0]);
            for (size_t i = 0; i < functionCount; i++) {
                if (strcmp(buffer->tokens[*current].value, functions[i]) == 0) {
                    isFunction = true;
                    break;
                }
            }
            
            if (!isFunction) {
                setError("Unexpected keyword in projection", 
                         buffer->tokens[*current].line,
                         buffer->tokens[*current].column,
                         buffer->tokens[*current].value);
                return false;
            }
            
            // Function call - expect opening parenthesis
            (*current)++;
            if (buffer->tokens[*current].type != TOKEN_DELIMITER ||
                strcmp(buffer->tokens[*current].value, "(") != 0) {
                setError("Expected '(' after function name", 
                         buffer->tokens[*current-1].line,
                         buffer->tokens[*current-1].column,
                         buffer->tokens[*current-1].value);
                return false;
            }
            (*current)++;

            // Function argument (column or *)
            if (buffer->tokens[*current].type != TOKEN_IDENTIFIER &&
                (buffer->tokens[*current].type != TOKEN_OPERATOR || 
                 strcmp(buffer->tokens[*current].value, "*") != 0)) {
                setError("Expected column or * in function", 
                         buffer->tokens[*current].line,
                         buffer->tokens[*current].column,
                         buffer->tokens[*current].value);
                return false;
            }
            (*current)++;

            // Closing parenthesis
            if (buffer->tokens[*current].type != TOKEN_DELIMITER ||
                strcmp(buffer->tokens[*current].value, ")") != 0) {
                setError("Expected ')' after function argument", 
                         buffer->tokens[*current-1].line,
                         buffer->tokens[*current-1].column,
                         buffer->tokens[*current-1].value);
                return false;
            }
            (*current)++;
        } else if (buffer->tokens[*current].type != TOKEN_IDENTIFIER) {
            setError("Expected column or function in projection", 
                     buffer->tokens[*current].line,
                     buffer->tokens[*current].column,
                     buffer->tokens[*current].value);
            return false;
        } else {
            // Simple column name
            (*current)++;
        }

        // Optional AS keyword and alias
        if (*current < buffer->count && 
            buffer->tokens[*current].type == TOKEN_KEYWORD &&
            strcmp(buffer->tokens[*current].value, "AS") == 0) {
            (*current)++;
            
            // Verify alias
            if (*current >= buffer->count || 
                buffer->tokens[*current].type != TOKEN_IDENTIFIER) {
                setError("Expected identifier after AS", 
                         buffer->tokens[*current-1].line,
                         buffer->tokens[*current-1].column,
                         buffer->tokens[*current-1].value);
                return false;
            }
            (*current)++;
        }

        // Check for comma or break if FROM encountered
        if (*current >= buffer->count) break;

        if (buffer->tokens[*current].type == TOKEN_DELIMITER &&
            strcmp(buffer->tokens[*current].value, ",") == 0) {
            (*current)++;
        } else if (buffer->tokens[*current].type == TOKEN_KEYWORD &&
                   strcmp(buffer->tokens[*current].value, "FROM") == 0) {
            break;
        }
    }

    // FROM keyword
    if (buffer->tokens[*current].type != TOKEN_KEYWORD ||
        strcmp(buffer->tokens[*current].value, "FROM") != 0) {
        setError("Expected FROM keyword", 
                 buffer->tokens[*current].line,
                 buffer->tokens[*current].column,
                 buffer->tokens[*current].value);
        return false;
    }
    (*current)++;

    // Table name or join
    if (*current >= buffer->count || 
        buffer->tokens[*current].type != TOKEN_IDENTIFIER) {
        setError("Expected table name", 
                 buffer->tokens[*current].line,
                 buffer->tokens[*current].column,
                 buffer->tokens[*current].value);
        return false;
    }
    (*current)++;

    // Optional JOIN clause
    while (*current < buffer->count) {
        if (buffer->tokens[*current].type == TOKEN_KEYWORD &&
            strcmp(buffer->tokens[*current].value, "JOIN") == 0) {
            (*current)++;
            
            // Table name
            if (*current >= buffer->count || 
                buffer->tokens[*current].type != TOKEN_IDENTIFIER) {
                setError("Expected table name after JOIN", 
                         buffer->tokens[*current-1].line,
                         buffer->tokens[*current-1].column,
                         buffer->tokens[*current-1].value);
                return false;
            }
            (*current)++;

            // ON keyword
            if (*current >= buffer->count || 
                buffer->tokens[*current].type != TOKEN_KEYWORD ||
                strcmp(buffer->tokens[*current].value, "ON") != 0) {
                setError("Expected ON keyword", 
                         buffer->tokens[*current-1].line,
                         buffer->tokens[*current-1].column,
                         buffer->tokens[*current-1].value);
                return false;
            }
            (*current)++;

            // Join condition
            // Simplified join condition parsing
            while (*current < buffer->count && 
                   buffer->tokens[*current].type != TOKEN_KEYWORD) {
                (*current)++;
            }
        } else {
            break;
        }
    }

    // Remaining clauses (WHERE, GROUP BY, HAVING, ORDER BY)
    while (*current < buffer->count) {
        Token currentToken = buffer->tokens[*current];
        
        if (currentToken.type == TOKEN_KEYWORD) {
            if (strcmp(currentToken.value, "WHERE") == 0) {
                // Start parsing WHERE clause
                (*current)++;
                if (!parseWhereClause(buffer, current)) {
                    return false;
                }
            } else if (strcmp(currentToken.value, "GROUP") == 0) {
                // Parse GROUP BY
                (*current)++;
                if (*current >= buffer->count || 
                    buffer->tokens[*current].type != TOKEN_KEYWORD ||
                    strcmp(buffer->tokens[*current].value, "BY") != 0) {
                    setError("Expected BY after GROUP", 
                             buffer->tokens[*current-1].line,
                             buffer->tokens[*current-1].column,
                             buffer->tokens[*current-1].value);
                    return false;
                }
                (*current)++;
                
                // Parse group by columns
                while (*current < buffer->count && 
                       buffer->tokens[*current].type != TOKEN_KEYWORD) {
                    (*current)++;
                }
            } else if (strcmp(currentToken.value, "HAVING") == 0) {
                // Parse HAVING clause
                (*current)++;
                while (*current < buffer->count && 
                       buffer->tokens[*current].type != TOKEN_KEYWORD) {
                    (*current)++;
                }
            } else if (strcmp(currentToken.value, "ORDER") == 0) {
                // Parse ORDER BY
                (*current)++;
                if (*current >= buffer->count || 
                    buffer->tokens[*current].type != TOKEN_KEYWORD ||
                    strcmp(buffer->tokens[*current].value, "BY") != 0) {
                    setError("Expected BY after ORDER", 
                             buffer->tokens[*current-1].line,
                             buffer->tokens[*current-1].column,
                             buffer->tokens[*current-1].value);
                    return false;
                }
                (*current)++;
                
                // Parse order by columns
                while (*current < buffer->count && 
                       buffer->tokens[*current].type != TOKEN_SEMICOLON) {
                    (*current)++;
                }
            } else {
                break;
            }
        } else {
            break;
        }
    }

    // Semicolon (optional)
    if (*current < buffer->count && 
        buffer->tokens[*current].type == TOKEN_SEMICOLON) {
        (*current)++;
    }

    return true;
}

bool parseWhereClause(TokenBuffer* buffer, int* current) {
    // Enhanced WHERE clause parsing
    while (*current < buffer->count) {
        // Column or table.column syntax
        if (buffer->tokens[*current].type == TOKEN_IDENTIFIER) {
            // Check for table.column syntax
            if (*current + 1 < buffer->count && 
                buffer->tokens[*current + 1].type == TOKEN_DELIMITER &&
                strcmp(buffer->tokens[*current + 1].value, ".") == 0) {
                (*current) += 2; // Skip table name and dot
            }
            
            // Expect an identifier at this point
            if (buffer->tokens[*current].type != TOKEN_IDENTIFIER) {
                setError("Invalid column reference", 
                         buffer->tokens[*current].line,
                         buffer->tokens[*current].column,
                         buffer->tokens[*current].value);
                return false;
            }
            (*current)++;
        }

        // More complex operator support (BETWEEN, comparison)
        if (buffer->tokens[*current].type == TOKEN_KEYWORD) {
            if (strcmp(buffer->tokens[*current].value, "BETWEEN") == 0) {
                // BETWEEN parsing
                (*current)++;
                // First value
                if (*current >= buffer->count || 
                    (buffer->tokens[*current].type != TOKEN_STRING && 
                     buffer->tokens[*current].type != TOKEN_INTEGER && 
                     buffer->tokens[*current].type != TOKEN_FLOAT)) {
                    setError("Expected value after BETWEEN", 
                             buffer->tokens[*current].line,
                             buffer->tokens[*current].column,
                             buffer->tokens[*current].value);
                    return false;
                }
                (*current)++;

                // AND keyword
                if (*current >= buffer->count || 
                    buffer->tokens[*current].type != TOKEN_KEYWORD ||
                    strcmp(buffer->tokens[*current].value, "AND") != 0) {
                    setError("Expected AND after first BETWEEN value", 
                             buffer->tokens[*current-1].line,
                             buffer->tokens[*current-1].column,
                             buffer->tokens[*current-1].value);
                    return false;
                }
                (*current)++;

                // Second value
                if (*current >= buffer->count || 
                    (buffer->tokens[*current].type != TOKEN_STRING && 
                     buffer->tokens[*current].type != TOKEN_INTEGER && 
                     buffer->tokens[*current].type != TOKEN_FLOAT)) {
                    setError("Expected value after AND in BETWEEN", 
                             buffer->tokens[*current].line,
                             buffer->tokens[*current].column,
                             buffer->tokens[*current].value);
                    return false;
                }
                (*current)++;
            }
        }

        // Comparison operators
        if (buffer->tokens[*current].type == TOKEN_OPERATOR) {
            (*current)++;
            // Value
            if (*current >= buffer->count || 
                (buffer->tokens[*current].type != TOKEN_STRING && 
                 buffer->tokens[*current].type != TOKEN_INTEGER && 
                 buffer->tokens[*current].type != TOKEN_FLOAT && 
                 buffer->tokens[*current].type != TOKEN_IDENTIFIER)) {
                setError("Expected value after comparison operator", 
                         buffer->tokens[*current-1].line,
                         buffer->tokens[*current-1].column,
                         buffer->tokens[*current-1].value);
                return false;
            }
            (*current)++;
        }

        // Logical operators
        if (*current < buffer->count && buffer->tokens[*current].type == TOKEN_KEYWORD) {
            if (strcmp(buffer->tokens[*current].value, "AND") == 0 || 
                strcmp(buffer->tokens[*current].value, "OR") == 0) {
                (*current)++;
            } else {
                break;  // Other keywords (FROM, GROUP, etc.) end WHERE
            }
        } else {
            break;
        }
    }

    return true;
}

void parseTokenBuffer(TokenBuffer* buffer) {
    int current = 0;

    while (current < buffer->count) {
        Token token = buffer->tokens[current];
        
        if (token.type == TOKEN_KEYWORD) {
            if (strcmp(token.value, "SELECT") == 0) {
                if (!parseSelectStatement(buffer, &current)) {
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