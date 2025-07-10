#ifndef EXPRESSION_RESULT_H
#define EXPRESSION_RESULT_H

typedef struct ExpressionResult
{
    char *type;
    char *c_code;  // Representação em código C para geração de código
    union
    {
        double doubleVal;
        int intVal;
        char *strVal;
    };
} ExpressionResult;

void free_expression_result(ExpressionResult* res);

#endif