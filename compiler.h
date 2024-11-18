#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>
#include <stdbool.h>
#include "semantic.h"

#define MAX_TOKEN_LENGTH 256
#define MAX_SYMBOLS 100
#define MAX_ERROR_LENGTH 512
#define MAX_KEYWORDS 50
#define INITIAL_TOKEN_BUFFER_SIZE 1024

// Token types
typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_OPERATOR,
    TOKEN_DELIMITER,
    TOKEN_KEYWORD,
    TOKEN_STRING,
    TOKEN_CHAR,
    TOKEN_COMMENT,
    TOKEN_SEMICOLON,
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    char value[MAX_TOKEN_LENGTH];
    int line;
    int column;
} Token;

// Token buffer structure
typedef struct {
    Token* tokens;
    int capacity;
    int count;
} TokenBuffer;

// Symbol table structure
typedef struct {
    char name[MAX_TOKEN_LENGTH];
    int id;
    char type[32];
    int scope;
    bool isDefined;  // Para verificação semântica
    char dataType[32];  // Para verificação de tipos
} Symbol;

// Error structure
typedef struct {
    const char* message;
    int line;
    int column;
    char context[MAX_ERROR_LENGTH];
} CompilerError;


// Function declarations
Token getNextToken(FILE *input, int *line, int *column);
TokenBuffer* createTokenBuffer(void);
void addTokenToBuffer(TokenBuffer* buffer, Token token);
void freeTokenBuffer(TokenBuffer* buffer);
int addSymbol(const char *name, const char *type, int scope);
int findSymbol(const char *name);
void printSymbolTable(void);
void parseTokenBuffer(TokenBuffer* buffer);
const char* getErrorMessage(void);
void clearError(void);
void compileSQL(const char* filename);
bool isKeyword(const char* str);
bool performSemanticAnalysis(TokenBuffer* buffer);
bool performSemanticAnalysis(TokenBuffer* buffer);
void addTable(SemanticContext* context, Table* table);


extern const char *TokenTypeNames[];
extern CompilerError currentError;
extern const char* SQL_KEYWORDS[];

#endif // COMPILER_H