#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>
#include <stdbool.h>
#include "semantic.h"
#include "parser.h"
#include "types.h"
#include "lexico.h"

// Function declarations
void compileSQL(const char* filename);

#endif // COMPILER_H