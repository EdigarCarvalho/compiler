#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "compiler.h"
#include "types.h"

void initSemanticContext(SemanticContext* context);
bool analyzeSemanticRules(SemanticContext* context);
void freeSemanticContext(SemanticContext* context);

#endif