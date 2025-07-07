#ifndef LVALUE_RESULT_H
#define LVALUE_RESULT_H

typedef enum {
    LVALUE_VAR,           // Simple variable: x
    LVALUE_ARRAY_ACCESS   // Array access: arr[0]
} LValueType;

typedef struct LValueResult {
    LValueType type;
    char* varName;        // For variables: the variable name
    char* elementType;    // For array access: the type of the element (e.g., "int")
} LValueResult;

LValueResult* create_lvalue_var(char* varName);
LValueResult* create_lvalue_array_access(char* varName, char* elementType);
void free_lvalue_result(LValueResult* lval);

#endif
