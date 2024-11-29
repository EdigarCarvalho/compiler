#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "types.h"
#include "semantic.h"
#include "intermediary.h"

void setError(const char* message, int line, int column, const char* context);
const char* getErrorMessage();
void clearError();
TokenBuffer* createTokenBuffer();
void addTokenToBuffer(TokenBuffer* buffer, Token token);
void freeTokenBuffer(TokenBuffer* buffer);
void parseTokenBuffer(TokenBuffer* buffer);
void addTable(SemanticContext* context, Table* table);
bool parseColumnList(TokenBuffer* buffer, int* current);
bool isSelectStatement(TokenBuffer* buffer, int* current);
bool parseWhereClause(TokenBuffer* buffer, int* current);
bool performSemanticAnalysis(TokenBuffer* buffer);

#endif