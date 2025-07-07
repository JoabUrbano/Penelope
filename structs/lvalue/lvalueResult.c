#include "lvalueResult.h"
#include <stdlib.h>
#include <string.h>

LValueResult* create_lvalue_var(char* varName) {
    LValueResult* result = malloc(sizeof(LValueResult));
    result->type = LVALUE_VAR;
    result->varName = strdup(varName);
    result->elementType = NULL;
    return result;
}

LValueResult* create_lvalue_array_access(char* varName, char* elementType) {
    LValueResult* result = malloc(sizeof(LValueResult));
    result->type = LVALUE_ARRAY_ACCESS;
    result->varName = strdup(varName);
    result->elementType = strdup(elementType);
    return result;
}

void free_lvalue_result(LValueResult* lval) {
    if (lval) {
        free(lval->varName);
        free(lval->elementType);
        free(lval);
    }
}
