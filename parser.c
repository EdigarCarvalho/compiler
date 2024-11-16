#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#define MAX_TOKEN_LENGTH 256
#define MAX_ERROR_LENGTH 512

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
    TOKEN_COMMENT,
    TOKEN_SEMICOLON
} TokenType;

typedef struct {
    TokenType type;
    char value[MAX_TOKEN_LENGTH];
    int line;
    int column;
} Token;

typedef struct {
    const char* message;
    int line;
    int column;
    char context[MAX_ERROR_LENGTH];
} ParseError;

// Global error state
ParseError currentError = {NULL, 0, 0, ""};

const char* tokenTypeToString(TokenType type) {
    switch(type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_KEYWORD: return "KEYWORD";
        case TOKEN_INTEGER: return "INTEGER";
        case TOKEN_FLOAT: return "FLOAT";
        case TOKEN_STRING: return "STRING";
        case TOKEN_CHAR: return "CHAR";
        case TOKEN_OPERATOR: return "OPERATOR";
        case TOKEN_DELIMITER: return "DELIMITER";
        case TOKEN_COMMENT: return "COMMENT";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        default: return "UNKNOWN";
    }
}

void setError(const char* message, int line, int column, const char* context) {
    currentError.message = message;
    currentError.line = line;
    currentError.column = column;
    strncpy(currentError.context, context, MAX_ERROR_LENGTH - 1);
    currentError.context[MAX_ERROR_LENGTH - 1] = '\0';
}

void clearError() {
    currentError.message = NULL;
    currentError.line = 0;
    currentError.column = 0;
    currentError.context[0] = '\0';
}

int isKeyword(const char* str) {
    const char* keywords[] = {
        "SELECT", "FROM", "WHERE", "IF", "THEN", "END",
        "INSERT", "UPDATE", "DELETE", "CREATE", "ALTER", "DROP",
        "JOIN", "ON", "GROUP", "BY", "HAVING", "ORDER", "LIMIT",
        "AND", "OR", "NOT", "IN", "BETWEEN", "LIKE", "IS", "NULL",
        "ASC", "DESC", "DISTINCT", "UNION", "ALL", "INTO", "VALUES",
        "SET", NULL
    };
    
    char upperStr[MAX_TOKEN_LENGTH];
    strncpy(upperStr, str, MAX_TOKEN_LENGTH - 1);
    upperStr[MAX_TOKEN_LENGTH - 1] = '\0';
    
    // Convert to upper case for case-insensitive comparison
    for(int i = 0; upperStr[i]; i++) {
        upperStr[i] = toupper(upperStr[i]);
    }
    
    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(upperStr, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

Token getNextToken(FILE *input, int *line, int *column) {
    Token token;
    token.line = *line;
    token.column = *column;
    token.value[0] = '\0';
    
    int c;
    int pos = 0;
    
    // Skip whitespace
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
    
    // Handle comments
    if (c == '-' && (c = fgetc(input)) == '-') {
        token.type = TOKEN_COMMENT;
        while ((c = fgetc(input)) != EOF && c != '\n') {
            if (pos < MAX_TOKEN_LENGTH - 1) {
                token.value[pos++] = c;
            }
        }
        if (c == '\n') {
            (*line)++;
            *column = 1;
        }
        token.value[pos] = '\0';
        return token;
    } else if (c == '/') {
        int next = fgetc(input);
        if (next == '*') {
            token.type = TOKEN_COMMENT;
            bool foundEnd = false;
            while ((c = fgetc(input)) != EOF) {
                if (c == '*') {
                    next = fgetc(input);
                    if (next == '/') {
                        foundEnd = true;
                        break;
                    }
                    ungetc(next, input);
                }
                if (c == '\n') {
                    (*line)++;
                    *column = 1;
                }
            }
            if (!foundEnd) {
                token.type = TOKEN_ERROR;
                setError("Unterminated multi-line comment", *line, *column, "/*...");
            }
            return token;
        }
        ungetc(next, input);
    }
    
    // Handle string literals
    if (c == '\'') {
        token.type = TOKEN_STRING;
        token.value[pos++] = c;
        bool escaped = false;
        while ((c = fgetc(input)) != EOF) {
            if (pos >= MAX_TOKEN_LENGTH - 1) {
                token.type = TOKEN_ERROR;
                setError("String literal too long", *line, *column, token.value);
                break;
            }
            
            if (c == '\n') {
                token.type = TOKEN_ERROR;
                setError("Unterminated string literal", *line, *column, token.value);
                break;
            }
            
            token.value[pos++] = c;
            
            if (c == '\\' && !escaped) {
                escaped = true;
            } else if (c == '\'' && !escaped) {
                break;
            } else {
                escaped = false;
            }
        }
        token.value[pos] = '\0';
        return token;
    }
    
    // Handle numbers
    if (isdigit(c)) {
        token.type = TOKEN_INTEGER;
        while (isdigit(c) || c == '.') {
            if (c == '.') {
                if (token.type == TOKEN_FLOAT) {
                    token.type = TOKEN_ERROR;
                    setError("Invalid number format: multiple decimal points", *line, *column, token.value);
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
    if (isalpha(c) || c == '_') {
        token.type = TOKEN_IDENTIFIER;
        while (isalnum(c) || c == '_') {
            token.value[pos++] = c;
            c = fgetc(input);
        }
        ungetc(c, input);
        token.value[pos] = '\0';
        
        if (isKeyword(token.value)) {
            token.type = TOKEN_KEYWORD;
        }
        return token;
    }
    
    // Handle operators and delimiters
    token.value[pos++] = c;
    token.value[pos] = '\0';
    
    switch (c) {
        case ';':
            token.type = TOKEN_SEMICOLON;
            break;
        case ',': case '(': case ')':
            token.type = TOKEN_DELIMITER;
            break;
        case '+': case '-': case '*': case '/':
        case '=': case '<': case '>': case '!':
            token.type = TOKEN_OPERATOR;
            // Check for two-character operators
            int next = fgetc(input);
            if ((c == '<' && next == '=') ||
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
            setError("Invalid character", *line, *column, token.value);
    }
    
    return token;
}

typedef struct {
    Token* tokens;
    int capacity;
    int count;
    int current;
} Parser;

void initParser(Parser* parser) {
    parser->capacity = 1024;
    parser->tokens = malloc(sizeof(Token) * parser->capacity);
    parser->count = 0;
    parser->current = 0;
}

void freeParser(Parser* parser) {
    free(parser->tokens);
}

void addToken(Parser* parser, Token token) {
    if (parser->count >= parser->capacity) {
        parser->capacity *= 2;
        parser->tokens = realloc(parser->tokens, sizeof(Token) * parser->capacity);
    }
    parser->tokens[parser->count++] = token;
}

Token peek(Parser* parser) {
    if (parser->current >= parser->count) {
        Token eof = {TOKEN_EOF, "EOF", 0, 0};
        return eof;
    }
    return parser->tokens[parser->current];
}

Token advance(Parser* parser) {
    if (parser->current >= parser->count) {
        Token eof = {TOKEN_EOF, "EOF", 0, 0};
        return eof;
    }
    return parser->tokens[parser->current++];
}

bool match(Parser* parser, TokenType type) {
    Token token = peek(parser);
    if (token.type == type) {
        advance(parser);
        return true;
    }
    return false;
}

void expect(Parser* parser, TokenType type, const char* errorMsg) {
    Token token = peek(parser);
    if (token.type != type) {
        setError(errorMsg, token.line, token.column, token.value);
        return;
    }
    advance(parser);
}

void parseSelectStatement(Parser* parser) {
    // SELECT
    expect(parser, TOKEN_KEYWORD, "Expected 'SELECT' keyword");
    if (currentError.message) return;
    
    // Column list
    do {
        expect(parser, TOKEN_IDENTIFIER, "Expected column name");
        if (currentError.message) return;
    } while (match(parser, TOKEN_DELIMITER)); // comma
    
    // FROM
    expect(parser, TOKEN_KEYWORD, "Expected 'FROM' keyword");
    if (currentError.message) return;
    
    // Table name
    expect(parser, TOKEN_IDENTIFIER, "Expected table name");
    if (currentError.message) return;
    
    // Optional WHERE clause
    if (match(parser, TOKEN_KEYWORD)) { // WHERE
        // Condition
        expect(parser, TOKEN_IDENTIFIER, "Expected column name in WHERE clause");
        if (currentError.message) return;
        
        expect(parser, TOKEN_OPERATOR, "Expected comparison operator");
        if (currentError.message) return;
        
        Token valueToken = peek(parser);
        if (valueToken.type != TOKEN_INTEGER && 
            valueToken.type != TOKEN_FLOAT && 
            valueToken.type != TOKEN_STRING && 
            valueToken.type != TOKEN_IDENTIFIER) {
            setError("Expected value in WHERE clause", valueToken.line, valueToken.column, valueToken.value);
            return;
        }
        advance(parser);
    }
    
    // Semicolon
    expect(parser, TOKEN_SEMICOLON, "Expected semicolon at end of statement");
}

void parseStatement(Parser* parser) {
    Token token = peek(parser);
    
    if (token.type == TOKEN_KEYWORD) {
        if (strcmp(token.value, "SELECT") == 0) {
            parseSelectStatement(parser);
        } else {
            setError("Unsupported SQL statement", token.line, token.column, token.value);
        }
    } else if (token.type != TOKEN_EOF && token.type != TOKEN_COMMENT) {
        setError("Expected SQL statement", token.line, token.column, token.value);
    }
}

void parseSQL(FILE *input) {
    Parser parser;
    initParser(&parser);
    
    int line = 1, column = 1;
    Token token;
    
    // Tokenization phase
    while ((token = getNextToken(input, &line, &column)).type != TOKEN_EOF) {
        if (token.type == TOKEN_ERROR) {
            printf("Lexical Error at line %d, column %d: %s\n", token.line, token.column, currentError.message);
            return;
        }
        if (token.type != TOKEN_COMMENT) { // Skip comments
            addToken(&parser, token);
        }
    }
    
    // Parsing phase
    while (parser.current < parser.count) {
        clearError();
        parseStatement(&parser);
        
        if (currentError.message) {
            printf("Syntax Error at line %d, column %d: %s\nNear: %s\n",
                   currentError.line, currentError.column,
                   currentError.message, currentError.context);
            break;
        }
    }
    
    freeParser(&parser);
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