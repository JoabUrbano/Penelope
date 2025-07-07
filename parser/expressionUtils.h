#ifndef EXPRESSION_UTILS_H
#define EXPRESSION_UTILS_H

#include "../structs/expression/expressionResult.h"

// Expression utility functions
char* expression_to_c_code(ExpressionResult* expr);
void print_value(ExpressionResult* expr);
void free_expression_result(ExpressionResult* expr);
double evaluate_number(char *str);

#endif // EXPRESSION_UTILS_H
