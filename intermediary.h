#ifndef INTERMEDIARY_H
#define INTERMEDIARY_H

#include "types.h"
#include <stdio.h>

#define MAX_INTERMEDIATE_CODE 1000

typedef struct {
    char op[20];
    char arg1[50];
    char arg2[50];
    char result[50];
} IntermediateCode;

IntermediateCode intermediateCode[MAX_INTERMEDIATE_CODE];
int intermediateCodeCount = 0;

void generateIntermediateCode(const char* op, const char* arg1, const char* arg2, const char* result);
void printIntermediateCode();


#endif 