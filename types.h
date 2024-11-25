#ifndef TYPES_H
#define TYPES_H

#include "stdbool.h"

#define MAX_NAME 64
#define MAX_TABLES 100
#define MAX_COLUMNS 50
#define MAX_JOINS 20
#define MAX_CONDITIONS 50
#define MAX_PROJECTIONS 50

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

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_VARCHAR,
    TYPE_DATE,
    TYPE_UNKNOWN
} DataType;

typedef struct {
    char name[MAX_NAME];
    DataType type;
    char table[MAX_NAME];
    bool isNullable;
} Column;

typedef struct {
    char name[MAX_NAME];
    Column columns[MAX_COLUMNS];
    int columnCount;
} Table;

typedef struct {
    char columnName[MAX_NAME];
    char tableName[MAX_NAME];
    Column* resolvedColumn;
} ColumnReference;

typedef struct {
    ColumnReference left;
    ColumnReference right;
    char operator[3];
} Condition;

typedef struct {
    char leftTable[MAX_NAME];
    char rightTable[MAX_NAME];
    Condition conditions[MAX_CONDITIONS];
    int conditionCount;
    char joinType[10];
} Join;

typedef struct {
    Table* tables[MAX_TABLES];
    int tableCount;
    Join joins[MAX_JOINS];
    int joinCount;
    ColumnReference projections[MAX_PROJECTIONS];
    int projectionCount;
    Condition whereConditions[MAX_CONDITIONS];
    int whereConditionCount;
    char* errors[100];
    int errorCount;
} SemanticContext;

extern const char *TokenTypeNames[];
extern CompilerError currentError;
extern const char* SQL_KEYWORDS[];


#endif