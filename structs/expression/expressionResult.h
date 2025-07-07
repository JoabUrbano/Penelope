#ifndef EXPRESSION_RESULT_H
#define EXPRESSION_RESULT_H

typedef struct ExpressionResult
{
    char *type;
    union
    {
        double doubleVal;
        int intVal;
        char *strVal;
    };
} ExpressionResult;

void free_expression_result(ExpressionResult* res);

#endif