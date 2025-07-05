#include "expressionResult.h"
#include <stdlib.h>
#include <string.h>

void free_expression_result(ExpressionResult* res) {
    if (!res) return;
    if (res->type) free(res->type);
    if (strcmp(res->type, "string") == 0 && res->strVal) free(res->strVal);
    free(res);
}