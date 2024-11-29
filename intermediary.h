#ifndef INTERMEDIATE_CODE_H
#define INTERMEDIATE_CODE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "types.h"

#define MAX_INTERMEDIATE_CODE 1000
#define MAX_OPERAND_LENGTH 100
#define MAX_OPERATOR_LENGTH 20

// Enum for intermediate code operation types
typedef enum
{
    IR_LOAD,       // Load table/data source
    IR_FROM,       // From table
    IR_SELECT,     // Filter rows
    IR_AS,         // Alias
    IR_CONDITIONS, // Multiple conditions
    IR_PROJECT,    // Select columns
    IR_AGGREGATE,  // Aggregate function
    IR_GROUP_BY,   // Group by operation
    IR_JOIN,       // Table join
    IR_ORDER_BY,   // Ordering results
    IR_CONST,      // Constant value
    IR_ASSIGNMENT, // Simple assignment
    IR_ARITHMETIC, // Arithmetic operation
    IR_RETURN,     // Return result set
    IR_CONCAT,     // Concatenation
    IR_BETWEEN,    // Between operation
    IR_HAVING,     // Having clause
} IntermediateCodeType;

// Struct to represent an intermediate code instruction
typedef struct
{
    IntermediateCodeType type;
    char result[MAX_OPERAND_LENGTH];
    char op1[MAX_OPERAND_LENGTH];
    char op2[MAX_OPERAND_LENGTH];
    char operation[MAX_OPERATOR_LENGTH];
} IntermediateCodeInstruction;

// Intermediate code generator context
typedef struct
{
    IntermediateCodeInstruction instructions[MAX_INTERMEDIATE_CODE];
    int instructionCount;
    int tempVarCounter;
} IntermediateCodeContext;

// Function prototypes
void initIntermediateCodeContext();
char *generateTempVar();
void generateIntermediateCode(TokenBuffer *buffer);
void addIntermediateCodeInstruction(
    IntermediateCodeType type,
    const char *result,
    const char *op1,
    const char *op2,
    const char *operation);
void printIntermediateCode();

#endif // INTERMEDIATE_CODE_H