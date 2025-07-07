#include "lvalueResult.h"
#include <stdlib.h>
#include <string.h>

LValueResult* create_lvalue_var(char* varName) {
    LValueResult* result = malloc(sizeof(LValueResult));
    result->type = LVALUE_VAR;
    result->varName = strdup(varName);
    result->elementType = NULL;
    result->dimensionCount = 0;
    result->indexExpressions = NULL;
    return result;
}

LValueResult* create_lvalue_array_access(char* varName, char* elementType) {
    LValueResult* result = malloc(sizeof(LValueResult));
    result->type = LVALUE_ARRAY_ACCESS;
    result->varName = strdup(varName);
    result->elementType = strdup(elementType);
    result->dimensionCount = 1;
    result->indexExpressions = malloc(sizeof(char*));
    result->indexExpressions[0] = strdup("0");
    return result;
}

LValueResult* create_lvalue_multidim_access(char* varName, char* elementType, int dimensions, char** indices) {
    LValueResult* result = malloc(sizeof(LValueResult));
    result->type = LVALUE_ARRAY_ACCESS;
    result->varName = strdup(varName);
    result->elementType = strdup(elementType);
    result->dimensionCount = dimensions;
    result->indexExpressions = malloc(dimensions * sizeof(char*));
    for (int i = 0; i < dimensions; i++) {
        result->indexExpressions[i] = strdup(indices[i]);
    }
    return result;
}

void free_lvalue_result(LValueResult* lval) {
    if (lval) {
        free(lval->varName);
        free(lval->elementType);
        if (lval->indexExpressions) {
            for (int i = 0; i < lval->dimensionCount; i++) {
                free(lval->indexExpressions[i]);
            }
            free(lval->indexExpressions);
        }
        free(lval);
    }
}
