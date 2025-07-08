#ifndef EXPRESSION_RESULT_H
#define EXPRESSION_RESULT_H

typedef struct ExpressionResult
{
    char *type;
    char *c_code;  // C code representation for code generation
    union
    {
        double doubleVal;
        int intVal;
        char *strVal;
    };
} ExpressionResult;

void free_expression_result(ExpressionResult* res);

#endif