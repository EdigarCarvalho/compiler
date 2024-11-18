#include "semantic.h"
#include <string.h>
#include <stdlib.h>

void initSemanticContext(SemanticContext* context) {
    context->tableCount = 0;
    context->joinCount = 0;
    context->projectionCount = 0;
    context->whereConditionCount = 0;
    context->errorCount = 0;
}

DataType getTokenDataType(TokenType type) {
    switch(type) {
        case TOKEN_INTEGER: return TYPE_INT;
        case TOKEN_FLOAT: return TYPE_FLOAT;
        case TOKEN_STRING: return TYPE_VARCHAR;
        default: return TYPE_UNKNOWN;
    }
}

Column* findColumn(SemanticContext* context, const char* tableName, const char* columnName) {
    for (int i = 0; i < context->tableCount; i++) {
        Table* table = context->tables[i];
        if (tableName[0] == '\0' || strcmp(table->name, tableName) == 0) {
            for (int j = 0; j < table->columnCount; j++) {
                if (strcmp(table->columns[j].name, columnName) == 0) {
                    return &table->columns[j];
                }
            }
        }
    }
    return NULL;
}

bool isTypeCompatible(DataType type1, DataType type2) {
    if (type1 == TYPE_UNKNOWN || type2 == TYPE_UNKNOWN) return true;
    if (type1 == type2) return true;
    if ((type1 == TYPE_INT && type2 == TYPE_FLOAT) ||
        (type1 == TYPE_FLOAT && type2 == TYPE_INT)) return true;
    return false;
}

void addSemanticError(SemanticContext* context, const char* error) {
    if (context->errorCount < 100) {
        context->errors[context->errorCount] = strdup(error);
        context->errorCount++;
    }
}

bool analyzeSemanticRules(TokenBuffer* buffer, SemanticContext* context) {
    bool isValid = true;

    // Example: Validate projections
    for (int i = 0; i < context->projectionCount; i++) {
        ColumnReference* proj = &context->projections[i];
        Column* col = findColumn(context, proj->tableName, proj->columnName);
        
        if (col == NULL) {
            char error[200];
            snprintf(error, sizeof(error), "Column not found: %s.%s", 
                     proj->tableName, proj->columnName);
            addSemanticError(context, error);
            isValid = false;
        }
    }

    // Add more semantic validation rules here
    return isValid;
}

void freeSemanticContext(SemanticContext* context) {
    for (int i = 0; i < context->errorCount; i++) {
        free(context->errors[i]);
    }
}