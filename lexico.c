#include "lexico.h"

const char *TokenTypeNames[] = {
    "IDENTIFIER",
    "INTEGER",
    "FLOAT",
    "OPERATOR",
    "DELIMITER",
    "KEYWORD",
    "STRING",
    "CHAR",
    "COMMENT",
    "SEMICOLON",
    "EOF",
    "ERROR"
};

const char* SQL_KEYWORDS[] = {
    "SELECT", "FROM", "WHERE", "INSERT", "UPDATE", "DELETE",
    "CREATE", "DROP", "TABLE", "DATABASE", "ALTER", "INDEX",
    "AND", "OR", "NOT", "IN", "BETWEEN", "LIKE", "IS", "NULL",
    "ORDER", "BY", "GROUP", "HAVING", "JOIN", "LEFT", "RIGHT",
    "INNER", "OUTER", "ON", "AS", "DISTINCT", "COUNT", "SUM",
    "AVG", "MAX", "MIN", "INTO", "VALUES", "SET", NULL
};

Symbol symbolTable[MAX_SYMBOLS];
int symbolCount = 0;
CompilerError currentError = {NULL, 0, 0, ""};

bool isKeyword(const char* str) {
    char upperStr[MAX_TOKEN_LENGTH];
    strncpy(upperStr, str, MAX_TOKEN_LENGTH - 1);
    upperStr[MAX_TOKEN_LENGTH - 1] = '\0';
    
    for(int i = 0; upperStr[i]; i++) {
        upperStr[i] = toupper(upperStr[i]);
    }
    
    for(int i = 0; SQL_KEYWORDS[i] != NULL; i++) {
        if(strcmp(upperStr, SQL_KEYWORDS[i]) == 0) {
            return true;
        }
    }
    return false;
}

int findSymbol(const char *name) {
    for(int i = 0; i < symbolCount; i++) {
        if(strcmp(symbolTable[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int addSymbol(const char *name, const char *type, int scope) {
    if(symbolCount >= MAX_SYMBOLS) {
        return -1;
    }
    if(findSymbol(name) != -1) {
        return findSymbol(name);
    }
    
    strcpy(symbolTable[symbolCount].name, name);
    strcpy(symbolTable[symbolCount].type, type);
    symbolTable[symbolCount].scope = scope;
    symbolTable[symbolCount].id = symbolCount + 1;
    return symbolCount++;
}

void printSymbolTable() {
    printf("\nSymbol Table:\n");
    printf("ID | Name                | Type      | Scope\n");
    printf("---|---------------------|-----------|-------\n");
    for(int i = 0; i < symbolCount; i++) {
        printf("%-3d| %-19s | %-9s | %d\n",
            symbolTable[i].id,
            symbolTable[i].name,
            symbolTable[i].type,
            symbolTable[i].scope);
    }
    printf("\n");
}

Token getNextToken(FILE *input, int *line, int *column) {
    Token token;
    token.line = *line;
    token.column = *column;
    token.value[0] = '\0';
    
    int c;
    int pos = 0;
    
    // Skip whitespace
    while((c = fgetc(input)) != EOF && isspace(c)) {
        if(c == '\n') {
            (*line)++;
            *column = 1;
        } else {
            (*column)++;
        }
    }
    
    if(c == EOF) {
        token.type = TOKEN_EOF;
        strcpy(token.value, "EOF");
        return token;
    }
    
    // Handle comments
    if(c == '-' && (c = fgetc(input)) == '-') {
        token.type = TOKEN_COMMENT;
        while((c = fgetc(input)) != EOF && c != '\n') {
            if(pos < MAX_TOKEN_LENGTH - 1) {
                token.value[pos++] = c;
            }
        }
        if(c == '\n') {
            (*line)++;
            *column = 1;
        }
        token.value[pos] = '\0';
        return token;
    }
    
    // Handle string literals
    if(c == '\'' || c == '"') {
        char delimiter = c;
        token.type = TOKEN_STRING;
        token.value[pos++] = c;
        while((c = fgetc(input)) != EOF && c != delimiter) {
            if(pos >= MAX_TOKEN_LENGTH - 1) break;
            if(c == '\n') {
                (*line)++;
                *column = 1;
            }
            token.value[pos++] = c;
        }
        if(c == delimiter) {
            token.value[pos++] = c;
        }
        token.value[pos] = '\0';
        return token;
    }
    
    // Handle numbers
    if(isdigit(c)) {
        token.type = TOKEN_INTEGER;
        while(isdigit(c) || c == '.') {
            if(c == '.') {
                if(token.type == TOKEN_FLOAT) {
                    token.type = TOKEN_ERROR;
                    break;
                }
                token.type = TOKEN_FLOAT;
            }
            token.value[pos++] = c;
            c = fgetc(input);
        }
        ungetc(c, input);
        token.value[pos] = '\0';
        return token;
    }
    
    // Handle identifiers and keywords
    if(isalpha(c) || c == '_') {
        token.type = TOKEN_IDENTIFIER;
        while(isalnum(c) || c == '_') {
            token.value[pos++] = c;
            c = fgetc(input);
        }
        ungetc(c, input);
        token.value[pos] = '\0';
        
        if(isKeyword(token.value)) {
            token.type = TOKEN_KEYWORD;
        } else {
            addSymbol(token.value, "IDENTIFIER", 0);
        }
        return token;
    }
    
    // Handle operators and delimiters
    token.value[pos++] = c;
    token.value[pos] = '\0';
    
    switch(c) {
        case ';':
            token.type = TOKEN_SEMICOLON;
            break;
        case ',': case '(': case ')':
            token.type = TOKEN_DELIMITER;
            break;
        case '+': case '-': case '*': case '/':
        case '=': case '<': case '>': case '!':
            token.type = TOKEN_OPERATOR;
            int next = fgetc(input);
            if((c == '<' && next == '=') ||
               (c == '>' && next == '=') ||
               (c == '!' && next == '=') ||
               (c == '<' && next == '>')) {
                token.value[pos++] = next;
                token.value[pos] = '\0';
            } else {
                ungetc(next, input);
            }
            break;
        default:
            token.type = TOKEN_ERROR;
            sprintf(token.value, "Invalid character: %c", c);
    }
    
    return token;
}