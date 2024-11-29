#include "intermediary.h"

IntermediateCodeContext intermediateCodeContext;

void initIntermediateCodeContext()
{
  intermediateCodeContext.instructionCount = 0;
  intermediateCodeContext.tempVarCounter = 0;
}

char *generateTempVar()
{
  static char tempVar[MAX_OPERAND_LENGTH];
  snprintf(tempVar, sizeof(tempVar), "T%d",
           intermediateCodeContext.tempVarCounter++);
  return tempVar;
}

void addIntermediateCodeInstruction(
    IntermediateCodeType type,
    const char *result,
    const char *op1,
    const char *op2,
    const char *operation)
{
  if (intermediateCodeContext.instructionCount >= MAX_INTERMEDIATE_CODE)
  {
    printf("Error: Intermediate code buffer overflow\n");
    return;
  }

  IntermediateCodeInstruction *instr =
      &intermediateCodeContext.instructions[intermediateCodeContext.instructionCount++];

  instr->type = type;
  strncpy(instr->result, result, MAX_OPERAND_LENGTH - 1);
  strncpy(instr->op1, op1 ? op1 : "", MAX_OPERAND_LENGTH - 1);
  strncpy(instr->op2, op2 ? op2 : "", MAX_OPERAND_LENGTH - 1);
  strncpy(instr->operation, operation ? operation : "", MAX_OPERATOR_LENGTH - 1);
}

void printIntermediateCode()
{
  printf("Intermediate Code Generation:\n");
  printf("-----------------------------\n");

  for (int i = 0; i < intermediateCodeContext.instructionCount; i++)
  {
    IntermediateCodeInstruction *instr =
        &intermediateCodeContext.instructions[i];

    switch (instr->type)
    {
    case IR_LOAD:
      printf("%s = LOAD %s\n", instr->result, instr->op1);
      break;
    case IR_FROM:
      printf("%s = %s FROM %s\n", instr->result, instr->op1, instr->op2);
      break;
    case IR_SELECT:
      printf("%s = SELECT %s\n",
             instr->result, instr->op1);
      break;
    case IR_AS:
      printf("%s = %s AS %s\n",
             instr->result, instr->op1, instr->op2);
      break;
    case IR_CONDITIONS:
      printf("%s = %s%s %s\n",
             instr->result, instr->op1, instr->operation, instr->op2);
      break;
    case IR_PROJECT:
      printf("%s = PROJECT %s (%s)\n",
             instr->result, instr->op1, instr->op2);
      break;
    case IR_AGGREGATE:
      printf("%s = %s(%s)\n",
             instr->result, instr->operation, instr->op1);
      break;
    case IR_GROUP_BY:
      printf("%s = GROUP %s BY %s\n",
             instr->result, instr->op1, instr->op2);
      break;
    case IR_JOIN:
      printf("%s = JOIN %s ON %s\n",
             instr->result, instr->op1, instr->op2);
      break;
    case IR_ORDER_BY:
      printf("%s = ORDER %s BY %s\n",
             instr->result, instr->op1, instr->op2);
      break;
    case IR_CONST:
      printf("%s = CONST %s\n", instr->result, instr->op1);
      break;
    case IR_ASSIGNMENT:
      printf("%s = %s\n", instr->result, instr->op1);
      break;
    case IR_ARITHMETIC:
      printf("%s = %s %s %s\n",
             instr->result, instr->op1, instr->operation, instr->op2);
      break;
    case IR_RETURN:
      printf("RETURN %s\n", instr->result);
      break;
    case IR_CONCAT:
      printf("%s = %s %s\n", instr->result, instr->op1, instr->op2);
      break;
    case IR_BETWEEN:
      printf("%s = %s BETWEEN %s\n",
             instr->result, instr->op1, instr->op2);
      break;
    case IR_HAVING:
      printf("%s = HAVING %s\n", instr->result, instr->operation);
      break;
    }
  }
}

void generateIntermediateCode(TokenBuffer *buffer)
{

  initIntermediateCodeContext();

  int current = 0;
  char *currentResult = NULL;
  bool hasGroupBy = false;

  const char *aggregateFuncs[] = {
      "COUNT", "SUM", "AVG", "MAX", "MIN"};
  const int numAggregateFuncs = sizeof(aggregateFuncs) / sizeof(aggregateFuncs[0]);

  while (current < buffer->count)
  {
    Token token = buffer->tokens[current];

    if (token.type == TOKEN_KEYWORD)
    {
      if (strcmp(token.value, "SELECT") == 0)
      {

        current++;

        char projectionColumns[MAX_OPERAND_LENGTH] = "";
        bool firstColumn = true;

        bool isAggregate = false;
        bool isAs = false;
        bool isMultipleConditions = false;
        char previousResult[MAX_OPERAND_LENGTH] = "";

        currentResult = generateTempVar();
        addIntermediateCodeInstruction(
            IR_PROJECT,
            currentResult,
            "temp_table",
            projectionColumns,
            NULL);

        for (int i = 0; i < numAggregateFuncs; i++)
        {
          if (strcmp(buffer->tokens[current].value, aggregateFuncs[i]) == 0)
          {
            isAggregate = true;
          }
        }

        while (current < buffer->count &&
               ((buffer->tokens[current].type != TOKEN_KEYWORD) || isAggregate))
        {

          if (isAggregate)
          {
            char aggregateOperand[MAX_OPERAND_LENGTH] = "";
            strcpy(aggregateOperand, buffer->tokens[current].value);
            current++;

            if (current < buffer->count &&
                strcmp(buffer->tokens[current].value, "(") == 0)
            {
              strcat(aggregateOperand, buffer->tokens[current].value);
              current++;
            }

            if (current < buffer->count &&
                buffer->tokens[current].type == TOKEN_IDENTIFIER)
            {
              strcat(aggregateOperand, buffer->tokens[current].value);
              current++;
            }

            if (current < buffer->count &&
                strcmp(buffer->tokens[current].value, ")") == 0)
            {
              strcat(aggregateOperand, buffer->tokens[current].value);
              current++;
            }

            if (current < buffer->count &&
                strcmp(buffer->tokens[current].value, "AS") == 0)
            {
              current++;
              if (current < buffer->count &&
                  buffer->tokens[current].type == TOKEN_IDENTIFIER)
              {
                currentResult = generateTempVar();
                addIntermediateCodeInstruction(
                    IR_AS,
                    currentResult,
                    aggregateOperand,
                    buffer->tokens[current].value,
                    NULL);
              }
            }
          }
          else if (buffer->tokens[current].type == TOKEN_IDENTIFIER)
          {
            current++;
            if (current < buffer->count &&
                strcmp(buffer->tokens[current].value, "AS") == 0)
            {
              current++;
              isAs = true;
              if (current < buffer->count &&
                  buffer->tokens[current].type == TOKEN_IDENTIFIER)
              {
                currentResult = generateTempVar();
                addIntermediateCodeInstruction(
                    IR_AS,
                    currentResult,
                    buffer->tokens[current].value,
                    buffer->tokens[current - 2].value,
                    NULL);
              }
            }
            else
            {
              if (!firstColumn)
              {
                strcat(projectionColumns, ", ");
              }
              strcat(projectionColumns, buffer->tokens[current].value);
              firstColumn = false;

              current++;
            }
          }

          if (buffer->count > current + 1 && strcmp(buffer->tokens[current + 1].value, "FROM") == 0)
          {

            if (isMultipleConditions && (isAs || isAggregate))
            {
              char current[100];
              strcpy(current, currentResult);

              char *tempVar = generateTempVar();

              addIntermediateCodeInstruction(
                  IR_CONDITIONS,
                  tempVar,
                  previousResult,
                  current,
                  ",");

              currentResult = tempVar;
            }
            break;
          }

          current++;

          if (isMultipleConditions && (isAs || isAggregate))
          {
            char current[100];
            strcpy(current, currentResult);

            char *tempVar = generateTempVar();

            addIntermediateCodeInstruction(
                IR_CONDITIONS,
                tempVar,
                previousResult,
                current,
                ",");

            currentResult = tempVar;
          }

          if (isAs || isAggregate)
          {
            strcpy(previousResult, currentResult);
            isMultipleConditions = true;
          }

          isAs = false;
          isAggregate = false;

          for (int i = 0; i < numAggregateFuncs; i++)
          {
            if (strcmp(buffer->tokens[current].value, aggregateFuncs[i]) == 0)
            {
              isAggregate = true;
            }
          }
        }

        if (current < buffer->count &&
            strcmp(buffer->tokens[current].value, "FROM") != 0)
        {

          for (size_t i = 0; i < sizeof(aggregateFuncs) / sizeof(aggregateFuncs[0]); i++)
          {
            if (strcmp(buffer->tokens[current].value, aggregateFuncs[i]) == 0)
            {
              isAggregate = true;
              current++;

              if (current < buffer->count &&
                  strcmp(buffer->tokens[current].value, "(") == 0)
              {
                current++;
              }

              if (current < buffer->count &&
                  buffer->tokens[current].type == TOKEN_IDENTIFIER)
              {
                currentResult = generateTempVar();

                addIntermediateCodeInstruction(
                    IR_AGGREGATE,
                    currentResult,
                    buffer->tokens[current].value,
                    NULL,
                    aggregateFuncs[i]);
                current++;
              }

              if (current < buffer->count &&
                  strcmp(buffer->tokens[current].value, ")") == 0)
              {
                current++;
              }
              break;
            }
          }
        }

        strcpy(previousResult, currentResult);

        currentResult = generateTempVar();
        addIntermediateCodeInstruction(
            IR_SELECT,
            currentResult,
            previousResult,
            NULL,
            NULL);
      }
      else if (strcmp(token.value, "FROM") == 0)
      {

        current++;
        char previousResult[MAX_OPERAND_LENGTH] = "";

        strcpy(previousResult, currentResult);

        currentResult = generateTempVar();

        if (current < buffer->count &&
            buffer->tokens[current].type == TOKEN_IDENTIFIER)
        {
          addIntermediateCodeInstruction(
              IR_FROM,
              currentResult,
              previousResult,
              buffer->tokens[current].value,
              NULL);
        }
      }
      else if (strcmp(token.value, "JOIN") == 0)
      {
        current++;
        char previousPart[MAX_OPERAND_LENGTH] = "";
        strcpy(previousPart, currentResult);
        if (buffer->tokens[current].type == TOKEN_IDENTIFIER)
        {
          char *table1 = buffer->tokens[current].value;
          current++;
          if (strcmp(buffer->tokens[current].value, "ON") == 0)
          {
            current++;
            char *firstPart = buffer->tokens[current].value;
            current++;

            if (buffer->tokens[current].type == TOKEN_OPERATOR)
            {
              char *operator= buffer->tokens[current].value;
              current++;

              char *secondPart = buffer->tokens[current].value;

              char *joinResult = generateTempVar();
              addIntermediateCodeInstruction(
                  IR_ARITHMETIC,
                  joinResult,
                  firstPart,
                  secondPart,
                  operator);

              char previousResult[100];
              strcpy(previousResult, currentResult);
              currentResult = generateTempVar();

              addIntermediateCodeInstruction(
                  IR_JOIN,
                  currentResult,
                  table1,
                  previousResult,
                  NULL);

              strcpy(previousResult, currentResult);
              currentResult = generateTempVar();
            }
          }
        }
      }
      else if (strcmp(token.value, "WHERE") == 0)
      {

        current++;

        while (current < buffer->count)
        {
          if (buffer->tokens[current].type == TOKEN_IDENTIFIER)
          {

            current++;

            if (strcmp(buffer->tokens[current].value, "BETWEEN") == 0)
            {
              current++;
              char condition1[MAX_OPERAND_LENGTH] = "";
              char condition2[MAX_OPERAND_LENGTH] = "";

              if (buffer->tokens[current].type == TOKEN_STRING)
              {
                strcpy(condition1, buffer->tokens[current].value);
                current++;
              }

              if (current < buffer->count && strcmp(buffer->tokens[current].value, "AND") == 0)
              {
                current++;
                if (buffer->tokens[current].type == TOKEN_STRING)
                {
                  strcpy(condition2, buffer->tokens[current].value);

                  addIntermediateCodeInstruction(
                      IR_ARITHMETIC,
                      currentResult,
                      condition1,
                      condition2,
                      "AND");

                  char previousResult[100];

                  strcpy(previousResult, currentResult);

                  currentResult = generateTempVar();

                  addIntermediateCodeInstruction(
                      IR_BETWEEN,
                      currentResult,
                      previousResult,
                      currentResult,
                      NULL);

                  break;
                }
              }
            }
          }
        }
      }
      else if (strcmp(token.value, "GROUP") == 0)
      {

        current++;
        if (current < buffer->count &&
            strcmp(buffer->tokens[current].value, "BY") == 0)
        {
          current++;
          hasGroupBy = true;

          char groupColumns[MAX_OPERAND_LENGTH] = "";
          bool firstColumn = true;

          while (current < buffer->count &&
                 buffer->tokens[current].type == TOKEN_IDENTIFIER)
          {
            if (!firstColumn)
            {
              strcat(groupColumns, ", ");
            }
            strcat(groupColumns, buffer->tokens[current].value);
            firstColumn = false;

            if (current + 1 < buffer->count &&
                buffer->tokens[current + 1].type == TOKEN_IDENTIFIER)
            {
              current++;
            }
            else
            {
              break;
            }
          }

          char result[100];
          if (currentResult)
          {
            strcpy(result, currentResult);
          }

          char *groupResult = generateTempVar();

          addIntermediateCodeInstruction(
              IR_GROUP_BY,
              groupResult,
              result,
              groupColumns,
              NULL);
          currentResult = groupResult;
        }
      }
      else if (strcmp(token.value, "HAVING") == 0)
      {

        if (!hasGroupBy)
        {
          printf("Error: HAVING clause without GROUP BY\n");
          break;
        }

        char havingCondition[MAX_OPERAND_LENGTH * 10] = "";
        char aggregateOperand[MAX_OPERAND_LENGTH * 10] = "";
        char aggregateFunc[MAX_OPERATOR_LENGTH * 10] = "";
        char havingOperator[MAX_OPERATOR_LENGTH * 10] = "";
        char havingValue[MAX_OPERAND_LENGTH * 10] = "";

        hasGroupBy = false;
        current++;

        bool isAggregate = false;
        for (int i = 0; i < numAggregateFuncs; i++)
        {
          if (strcmp(buffer->tokens[current].value, aggregateFuncs[i]) == 0)
          {
            isAggregate = true;
          }
        }

        while (current < buffer->count &&
               (buffer->tokens[current].type != TOKEN_KEYWORD || isAggregate))
        {

          bool isAggregateFunc = false;
          for (int i = 0; i < numAggregateFuncs; i++)
          {
            if (strcmp(buffer->tokens[current].value, aggregateFuncs[i]) == 0)
            {
              isAggregateFunc = true;
              strcpy(aggregateFunc, aggregateFuncs[i]);
              current++;
              break;
            }
          }

          if (isAggregateFunc)
          {

            if (current < buffer->count &&
                strcmp(buffer->tokens[current].value, "(") == 0)
            {
              current++;
            }

            if (current < buffer->count &&
                buffer->tokens[current].type == TOKEN_IDENTIFIER)
            {
              strcpy(aggregateOperand, buffer->tokens[current].value);
              current++;
            }

            if (current < buffer->count &&
                strcmp(buffer->tokens[current].value, ")") == 0)
            {
              current++;
            }
          }

          if (strcmp(buffer->tokens[current].value, ">") == 0 ||
              strcmp(buffer->tokens[current].value, "<") == 0 ||
              strcmp(buffer->tokens[current].value, ">=") == 0 ||
              strcmp(buffer->tokens[current].value, "<=") == 0 ||
              strcmp(buffer->tokens[current].value, "=") == 0)
          {
            strcpy(havingOperator, buffer->tokens[current].value);
            current++;

            if (current < buffer->count &&
                (buffer->tokens[current].type == TOKEN_FLOAT || buffer->tokens[current].type == TOKEN_INTEGER ||
                 buffer->tokens[current].type == TOKEN_STRING))
            {
              strcpy(havingValue, buffer->tokens[current].value);
              current++;
            }
          }

          isAggregate = false;

          for (int i = 0; i < numAggregateFuncs; i++)
          {
            if (strcmp(buffer->tokens[current].value, aggregateFuncs[i]) == 0)
            {
              isAggregate = true;
            }
          }
        }

        if (strlen(aggregateFunc) > 0 && strlen(aggregateOperand) > 0)
        {
          char *aggregateHavingResult = generateTempVar();
          addIntermediateCodeInstruction(
              IR_AGGREGATE,
              aggregateHavingResult,
              aggregateOperand,
              NULL,
              aggregateFunc);

          strcat(havingCondition, aggregateHavingResult);
          strcat(havingCondition, " ");
          strcat(havingCondition, havingOperator);
          strcat(havingCondition, " ");
          strcat(havingCondition, buffer->tokens[current - 1].value);

          char *havingResult = generateTempVar();

          addIntermediateCodeInstruction(
              IR_HAVING,
              havingResult,
              currentResult,
              aggregateHavingResult,
              havingCondition);
          currentResult = havingResult;
        }
      }
    }

    current++;
  }

  if (currentResult)
  {
    addIntermediateCodeInstruction(
        IR_RETURN,
        currentResult,
        NULL,
        NULL,
        NULL);
  }
}
