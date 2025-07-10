#ifndef LVALUE_RESULT_H
#define LVALUE_RESULT_H

typedef enum {
    LVALUE_VAR,           // Variável simples: x
    LVALUE_ARRAY_ACCESS,  // Acesso a array: arr[0] ou arr[i][j]
    LVALUE_STRUCT_FIELD   // Acesso a campo de struct: person.name
} LValueType;

typedef struct LValueResult {
    LValueType type;
    char* varName;        // Para variáveis: o nome da variável
    char* elementType;    // Para acesso a array: o tipo do elemento (ex: "int")
    int dimensionCount;   // Número de dimensões (1 para arr[i], 2 para arr[i][j])
    char** indexExpressions; // Expressões C para cada índice
    // Para acesso a campo de struct
    char* fieldName;      // Nome do campo (para LVALUE_STRUCT_FIELD)
    char* structType;     // Tipo do struct (para LVALUE_STRUCT_FIELD)
} LValueResult;

LValueResult* create_lvalue_var(char* varName);
LValueResult* create_lvalue_array_access(char* varName, char* elementType);
LValueResult* create_lvalue_multidim_access(char* varName, char* elementType, int dimensions, char** indices);
LValueResult* create_lvalue_struct_field(char* varName, char* fieldName, char* structType, char* fieldType);
void free_lvalue_result(LValueResult* lval);

#endif
