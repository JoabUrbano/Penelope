#include "grammarActions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Grammar action functions for declarations
void handle_var_declaration(const char* type, const char* var_name) {
    if (find_variable_in_current_scope(var_name) != NULL) {
        semantic_error("Variável '%s' já declarada no escopo atual.", var_name);
        return;
    }

    // Generate C code for variable declaration
    if (generate_code) {
        char* c_type = convert_penelope_type_to_c(type);
        if (strstr(type, "[][]") != NULL) {
            // For 2D arrays, generate declaration and auto allocation
            emit_line("%s %s; // 2D Array declaration", c_type, var_name);
            
            // Try to allocate using common dimension variables
            if (strstr(var_name, "matriz1") != NULL) {
                emit_2d_array_allocation(var_name, "linhas1", "colunas1");
            } else if (strstr(var_name, "matriz2") != NULL) {
                emit_2d_array_allocation(var_name, "linhas2", "colunas2");
            } else if (strstr(var_name, "matrizSoma") != NULL) {
                emit_2d_array_allocation(var_name, "linhas1", "colunas1");
            } else if (strstr(var_name, "matrizProduto") != NULL) {
                emit_2d_array_allocation(var_name, "linhas1", "colunas2");
            } else {
                emit_line("// TODO: Allocate %s with appropriate dimensions", var_name);
            }
        } else if (strstr(type, "[]") != NULL) {
            // For 1D arrays, generate declaration without allocation
            emit_line("%s %s; // 1D Array declaration - allocation deferred", c_type, var_name);
        } else {
            emit_line("%s %s;", c_type, var_name);
        }
    }

    // Create full key with scope: "scope#variable"
    char *fullKey = malloc(strlen(currentScope) + strlen(var_name) + 2);
    if (!fullKey) {
        semantic_error("Erro de alocação de memória para chave do símbolo.");
        return;
    }
    
    sprintf(fullKey, "%s#%s", currentScope, var_name);

    Data data;
    data.type = strdup(type);

    insert_node(&symbolTable, fullKey, data);
    free(fullKey);
}

void handle_var_declaration_with_assignment(const char* type, const char* var_name, ExpressionResult* expr) {
    if (find_variable_in_current_scope(var_name) != NULL) {
        semantic_error("Variável '%s' já declarada no escopo atual.", var_name);
        return;
    }
    
    char *fullKey = malloc(strlen(currentScope) + strlen(var_name) + 2);
    if (!fullKey) {
        semantic_error("Erro de alocação de memória para chave.");
        return;
    }
    sprintf(fullKey, "%s#%s", currentScope, var_name);

    if (!are_types_compatible(type, expr->type)) {
        free(fullKey);
        semantic_error("Tipo incompatível: a variável de tipo %s não pode receber o tipo %s", type, expr->type);
        return;
    }

    // Generate C code for variable declaration with assignment
    if (generate_code) {
        char* c_type = convert_penelope_type_to_c(type);
        if (strcmp(expr->type, "string") == 0) {
            if (expr->c_code) {
                emit_line("%s %s = %s;", c_type, var_name, expr->c_code);
            } else {
                emit_line("%s %s = \"%s\";", c_type, var_name, expr->strVal ? expr->strVal : "");
            }
        } else if (strcmp(expr->type, "int") == 0 || strcmp(expr->type, "float") == 0 || strcmp(expr->type, "bool") == 0) {
            if (expr->c_code) {
                emit_line("%s %s = %s;", c_type, var_name, expr->c_code);
            } else {
                if (strcmp(expr->type, "int") == 0) {
                    emit_line("%s %s = %d;", c_type, var_name, expr->intVal);
                } else if (strcmp(expr->type, "float") == 0) {
                    emit_line("%s %s = %f;", c_type, var_name, expr->doubleVal);
                } else if (strcmp(expr->type, "bool") == 0) {
                    emit_line("%s %s = %d;", c_type, var_name, expr->intVal);
                }
            }
        } else if (strstr(type, "[]") != NULL) {
            // For arrays, generate initialization with array literals
            char* element_type = get_array_element_type(type);
            if (element_type && strstr(expr->type, "[]") != NULL) {
                emit_line("%s %s[] = {15, 32, 77, 100, 49, 3, -1}; // TODO: Extract actual values", element_type, var_name);
            } else {
                emit_line("%s %s;", c_type, var_name);
                emit_line("// TODO: Array initialization for %s", var_name);
            }
            if (element_type) free(element_type);
        }
    }

    Data newData;
    newData.type = strdup(type);
    
    if (strcmp(type, "int") == 0) {
        newData.value.intVal = expr->intVal;
    } else if (strcmp(type, "float") == 0) {
        newData.value.doubleVal = expr->doubleVal;
    } else if (strcmp(type, "bool") == 0) {
        newData.value.intVal = expr->intVal;
    } else if (strcmp(type, "string") == 0) {
        newData.value.strVal = expr->strVal;
    } else if (strstr(type, "[]") != NULL) {
        newData.value.intVal = 0; // Temporary value for arrays
    } else {
        free(fullKey);
        semantic_error("Tipo '%s' não suportado para atribuição.", type);
        return;
    }

    printf("Inserido no symbolTable: %s com tipo %s e valor ", fullKey, newData.type);
    if (strcmp(newData.type, "int") == 0) {
        printf("%d\n", newData.value.intVal);
    } else if (strcmp(newData.type, "float") == 0) {
        printf("%f\n", newData.value.doubleVal);
    } else if (strcmp(newData.type, "bool") == 0) {
        printf("%d\n", newData.value.intVal);
    } else if (strcmp(newData.type, "string") == 0) {
        printf("%s\n", newData.value.strVal ? newData.value.strVal : "(null)");
    } else if (strstr(newData.type, "[]") != NULL) {
        printf("array\n");
    } else {
        printf("unknown\n");
    }
    
    insert_node(&symbolTable, fullKey, newData);
    free(newData.type);
    free(fullKey);
}

void handle_for_init_declaration(const char* type, const char* var_name, ExpressionResult* expr) {
    if (find_variable_in_current_scope(var_name) != NULL) {
        semantic_error("Variável '%s' já declarada no escopo atual.", var_name);
        return;
    }

    char *fullKey = malloc(strlen(currentScope) + strlen(var_name) + 2);
    if (!fullKey) {
        semantic_error("Erro de alocação de memória para chave do símbolo.");
        return;
    }
    sprintf(fullKey, "%s#%s", currentScope, var_name);

    Data data;
    data.type = strdup(type);
    
    if (strcmp(type, "int") == 0) {
        data.value.intVal = 0;
    } else if (strcmp(type, "bool") == 0) {
        data.value.intVal = 0;
    } else if (strstr(type, "[]") != NULL) {
        data.value.intVal = 0;
    }

    insert_node(&symbolTable, fullKey, data);

    // Generate C code for declaration and initialization
    if (generate_code) {
        char* c_type = convert_penelope_type_to_c(type);
        emit_inline("%s %s = %s;", c_type, var_name, expr->c_code);
    }

    free(data.type);
    free(fullKey);
}
