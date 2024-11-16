#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_TOKEN_LENGTH 256

typedef enum {
    TOKEN_EOF,
    TOKEN_ERROR,
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_CHAR,
    TOKEN_OPERATOR,
    TOKEN_DELIMITER,
    TOKEN_COMMENT
} TokenType;

typedef struct {
    TokenType type;
    char value[MAX_TOKEN_LENGTH];
    int line;
    int column;
} Token;

void reportSyntaxError(const char* expected, Token token) {
    printf("Error: Expected '%s' at line %d, column %d, but got '%s'.\n", 
            expected, token.line, token.column, token.value);
}

int isKeyword(const char* str) {
    const char* keywords[] = {
        "SELECT", "FROM", "WHERE", "IF", "THEN", "END",
        "INSERT", "UPDATE", "DELETE", "CREATE", "ALTER", "DROP",
        "JOIN", "ON", "GROUP", "BY", "HAVING", "ORDER", "LIMIT", "UNION", NULL
    };
    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(str, keywords[i]) == 0) {
            return 1; // It's a keyword
        }
    }
    return 0; // Not a keyword
}

Token getNextToken(FILE *input, int *line, int *column) {
    Token token;
    token.line = *line;
    token.column = *column;
    token.value[0] = '\0';

    int c;
    int pos = 0;

    // Ignore whitespace
    while ((c = fgetc(input)) != EOF && isspace(c)) {
        if (c == '\n') {
            (*line)++;
            *column = 1;
        } else {
            (*column)++;
        }
    }

    if (c == EOF) {
        token.type = TOKEN_EOF;
        strcpy(token.value, "EOF");
        return token;
    }

    // Identify comments
    if (c == '-') {
        int nextChar = fgetc(input);
        (*column)++;
        if (nextChar == '-') { // Single-line comment
            token.type = TOKEN_COMMENT;
            token.value[pos++] = c;
            token.value[pos++] = nextChar;
            while ((c = fgetc(input)) != EOF && c != '\n') {
                token.value[pos++] = c;
                (*column)++;
            }
            token.value[pos] = '\0';
            if (c == '\n') {
                (*line)++;
                *column = 1;
            }
            return token;
        } else {
            ungetc(nextChar, input);
        }
    } else if (c == '/') {
        int nextChar = fgetc(input);
        (*column)++;
        if (nextChar == '*') { // Multi-line comment
            token.type = TOKEN_COMMENT;
            token.value[pos++] = c;
            token.value[pos++] = nextChar;
            while ((c = fgetc(input)) != EOF) {
                token.value[pos++] = c;
                (*column)++;
                if (c == '*') {
                    int nextNextChar = fgetc(input);
                    (*column)++;
                    if (nextNextChar == '/') {
                        token.value[pos++] = nextNextChar;
                        break;
                    } else {
                        ungetc(nextNextChar, input);
                    }
                } else if (c == '\n') {
                    (*line)++;
                    *column = 1;
                }
            }
            if (c == EOF) {
                token.type = TOKEN_ERROR;
                strcpy(token.value, "Unterminated comment");
            }
            token.value[pos] = '\0';
            return token;
        } else {
            ungetc(nextChar, input);
        }
    }

    // Identifiers and keywords
    if (isalpha(c)) {
        token.type = TOKEN_IDENTIFIER;
        while (isalnum(c)) {
            token.value[pos++] = c;
            c = fgetc(input);
            (*column)++;
        }
        ungetc(c, input);
        token.value[pos] = '\0';

        // Check if it's a keyword
        if (isKeyword(token.value)) {
            token.type = TOKEN_KEYWORD;
        }
    }
    // Numbers (integers and floats)
    else if (isdigit(c)) {
        token.type = TOKEN_INTEGER;
        while (isdigit(c)) {
            token.value[pos++] = c;
            c = fgetc(input);
            (*column)++;
        }
        if (c == '.') {
            token.type = TOKEN_FLOAT;
            token.value[pos++] = c;
            c = fgetc(input);
            (*column)++;
            while (isdigit(c)) {
                token.value[pos++] = c;
                c = fgetc(input);
                (*column)++;
            }
        }
        ungetc(c, input);
        token.value[pos] = '\0';
    }
    // Strings
    else if (c == '"') {
        token.type = TOKEN_STRING;
        token.value[pos++] = c;
        while ((c = fgetc(input)) != EOF && c != '"') {
            token.value[pos++] = c;
            (*column)++;
        }
        if (c == '"') {
            token.value[pos++] = c;
        } else {
            token.type = TOKEN_ERROR;
            strcpy(token.value, "Unterminated string");
        }
        token.value[pos] = '\0';
    }
    // Characters
    else if (c == '\'') {
        token.type = TOKEN_CHAR;
        token.value[pos++] = c;
        c = fgetc(input);
        if (c != EOF && c != '\'') {
            token.value[pos++] = c;
            c = fgetc(input);
            if (c == '\'') {
                token.value[pos++] = c;
            } else {
                token.type = TOKEN_ERROR;
                strcpy(token.value, "Unterminated character");
            }
        } else {
            token.type = TOKEN_ERROR;
            strcpy(token.value, "Invalid character literal");
        }
        token.value[pos] = '\0';
    }
    // Operators and delimiters
    else {
        token.value[pos++] = c;
        (*column)++;
        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || c == '<' || c == '>' || c == '!') {
            token.type = TOKEN_OPERATOR;
        } else if (c == ',' || c == ';' || c == '(' || c == ')') {
            token.type = TOKEN_DELIMITER;
        } else {
            token.type = TOKEN_ERROR;
            sprintf(token.value, "Unexpected character '%c'", c);
        }
        token.value[pos] = '\0';
    }

    return token;
}

void parseSQL(FILE *input) {
    Token token;
    int line = 1, column = 1;
    
    while ((token = getNextToken(input, &line, &column)).type != TOKEN_EOF) {
        if (token.type == TOKEN_ERROR) {
            printf("Error at line %d, column %d: %s\n", token.line, token.column, token.value);
            return;
        }
        printf("Token: %d, Value: %s, Line: %d, Column: %d\n", token.type, token.value, token.line, token.column);
    }
}

int main() {
    FILE *input = fopen("test.sql", "r");
    if (input == NULL) {
        printf("Error opening file.\n");
        return 1;
    }
    parseSQL(input);
    fclose(input);
    return 0;
}
