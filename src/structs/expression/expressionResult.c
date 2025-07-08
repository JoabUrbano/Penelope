#include "expressionResult.h"
#include <stdlib.h>
#include <string.h>

void free_expression_result(ExpressionResult* res) {
    if (!res) return;
    if (res->type && strcmp(res->type, "string") == 0 && res->strVal) {
        free(res->strVal);
        res->strVal = NULL; // Evita double free
    }
    if (res->type) {
        free(res->type);
        res->type = NULL;
    }
    if (res->c_code) {
        free(res->c_code);
        res->c_code = NULL;
    }
    free(res);
}