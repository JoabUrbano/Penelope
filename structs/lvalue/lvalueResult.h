#ifndef LVALUE_RESULT_H
#define LVALUE_RESULT_H

typedef enum {
    LVALUE_VAR,           // Variável simples: x
    LVALUE_ARRAY_ACCESS   // Acesso a array: arr[0]
} LValueType;

typedef struct LValueResult {
    LValueType type;
    char* varName;        // Para variáveis: o nome da variável
    char* elementType;    // Para acesso a array: o tipo do elemento (ex: "int")
} LValueResult;

LValueResult* create_lvalue_var(char* varName);
LValueResult* create_lvalue_array_access(char* varName, char* elementType);
void free_lvalue_result(LValueResult* lval);

#endif
