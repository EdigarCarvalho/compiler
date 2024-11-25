#ifndef LEXICO_H
#define LEXICO_H

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"

bool isKeyword(const char* str);
int findSymbol(const char *name);
int addSymbol(const char *name, const char *type, int scope);
void printSymbolTable(void);
Token getNextToken(FILE *input, int *line, int *column);

#endif