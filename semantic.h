#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "compiler.h"

#define MAX_NAME 64
#define MAX_TABLES 100
#define MAX_COLUMNS 50
#define MAX_JOINS 20
#define MAX_CONDITIONS 50
#define MAX_PROJECTIONS 50

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

void initSemanticContext(SemanticContext* context);
bool analyzeSemanticRules(TokenBuffer* buffer, SemanticContext* context);
void freeSemanticContext(SemanticContext* context);

#endif