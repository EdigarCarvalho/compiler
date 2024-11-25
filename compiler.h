#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>
#include <stdbool.h>
#include "semantic.h"
#include "types.h"

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