#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_TOKEN_LENGTH 100
#define MAX_SYMBOLS 100

// Token types
enum TokenType {
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_OPERATOR,
    TOKEN_DELIMITER,
    TOKEN_KEYWORD,
    TOKEN_STRING,
    TOKEN_CHAR,
    TOKEN_COMMENT,
    TOKEN_EOF,
    TOKEN_ERROR
};

const char *TokenTypeNames[] = {
    "TOKEN_IDENTIFIER",
    "TOKEN_INTEGER",
    "TOKEN_FLOAT",
    "TOKEN_OPERATOR",
    "TOKEN_DELIMITER",
    "TOKEN_KEYWORD",
    "TOKEN_STRING",
    "TOKEN_CHAR",
    "TOKEN_COMMENT",
    "TOKEN_EOF",
    "TOKEN_ERROR"
};

// Token structure
typedef struct {
    enum TokenType type;
    char value[MAX_TOKEN_LENGTH];
    int line;
    int column;
} Token;

// Symbol table structure
typedef struct {
    char name[MAX_TOKEN_LENGTH];
    int id;
} Symbol;

Symbol symbolTable[MAX_SYMBOLS];
int symbolCount = 0;

// Function to check if a symbol is already in the table
int findSymbol(const char *name) {
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(symbolTable[i].name, name) == 0) {
            return i; // Return the index in the table
        }
    }
    return -1; // Not found
}

// Function to add a symbol to the table
int addSymbol(const char *name) {
    if (symbolCount >= MAX_SYMBOLS) {
        printf("Error: Symbol table is full!\n");
        return -1;
    }
    strcpy(symbolTable[symbolCount].name, name);
    symbolTable[symbolCount].id = symbolCount + 1;
    return symbolCount++;
}

// Function to get the next token
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
if (strcmp(token.value, "SELECT") == 0 || strcmp(token.value, "FROM") == 0 ||
    strcmp(token.value, "WHERE") == 0 || strcmp(token.value, "IF") == 0 ||
    strcmp(token.value, "THEN") == 0 || strcmp(token.value, "END") == 0 ||
    strcmp(token.value, "INSERT") == 0 || strcmp(token.value, "UPDATE") == 0 ||
    strcmp(token.value, "DELETE") == 0 || strcmp(token.value, "CREATE") == 0 ||
    strcmp(token.value, "ALTER") == 0 || strcmp(token.value, "DROP") == 0 ||
    strcmp(token.value, "JOIN") == 0 || strcmp(token.value, "ON") == 0 ||
    strcmp(token.value, "GROUP") == 0 || strcmp(token.value, "BY") == 0 ||
    strcmp(token.value, "HAVING") == 0 || strcmp(token.value, "ORDER") == 0 ||
    strcmp(token.value, "LIMIT") == 0 || strcmp(token.value, "UNION") == 0) {
    token.type = TOKEN_KEYWORD;
} else {
            // Add identifier to the symbol table
            if (findSymbol(token.value) == -1) {
                addSymbol(token.value);
            }
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

// Function to print the symbol table
void printSymbolTable() {
    printf("\nSymbol Table:\n");
    printf("ID | Name\n");
    printf("---|--------------------\n");
    for (int i = 0; i < symbolCount; i++) {
        printf("%2d | %s\n", symbolTable[i].id, symbolTable[i].name);
    }
}

int main() {
    FILE *input = fopen("test.sql", "r");
    if (!input) {
        printf("Error opening the file!\n");
        return 1;
    }

    int line = 1, column = 1;
    Token token;

    printf("Tokens found:\n");
    do {
        token = getNextToken(input, &line, &column);
        printf("Token: { Type: %s, Value: '%s', Line: %d, Column: %d }\n",
               TokenTypeNames[token.type], token.value, token.line, token.column);
    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);

    fclose(input);

    // Print the symbol table
    printSymbolTable();

    return 0;
}