#include "grammarActions.h"
#include "../semantics/semantics.h"
#include "../codeGenerator/codeGenerator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _GNU_SOURCE  // para strdup

// Ações de Declaração de Variáveis
void handle_var_declaration(const char* type, const char* var_name) {
    // Análise semântica: validar e declarar variável
    if (!validate_variable_declaration(type, var_name)) {
        return; // Erro já reportado pela semântica
    }
    
    // Declarar na tabela de símbolos
    declare_variable(type, var_name, NULL);
    
    // Gerar código
    emit_var_declaration_code(type, var_name);
}

void handle_var_declaration_with_assignment(const char* type, const char* var_name, ExpressionResult* expr) {
    // Análise semântica: validar declaração e atribuição de variável
    if (!validate_variable_declaration(type, var_name)) {
        return;
    }
    
    if (!check_type_compatibility(type, expr->type)) {
        semantic_error("Tipo não compatível: Não é possível atribuir %s para a variável do tipo %s", expr->type, type);
        return;
    }
    
    // Declarar e inicializar na tabela de símbolos
    declare_variable(type, var_name, expr);
    
    // Gerar código
    emit_var_assignment_code(type, var_name, expr);
}

void handle_for_init_declaration(const char* type, const char* var_name, ExpressionResult* expr) {
    // Análise semântica: validar declaração e atribuição de variável
    if (!validate_variable_declaration(type, var_name)) {
        return;
    }
    
    if (!check_type_compatibility(type, expr->type)) {
        semantic_error("Type mismatch: cannot assign %s to variable of type %s", expr->type, type);
        return;
    }
    
    // Declarar e inicializar na tabela de símbolos
    declare_variable(type, var_name, expr);
    
    // Gerar código inline para for loop
    emit_for_init_code(type, var_name, expr);
}

// Ações de Controle de Fluxo
int handle_while_condition(ExpressionResult* condition) {
    // Análise semântica: validar condição
    if (!validate_condition_expression(condition)) {
        return -1; // Erro
    }
    
    // Gerar labels
    int start_label = generate_label();
    int end_label = generate_label();
    
    // Atualizar estado de execução
    exec_block = evaluate_boolean_expression(condition);
    current_loop_exit_label = end_label;
    
    // Gerar código
    emit_while_start_code(condition, start_label, end_label);
    
    return start_label;
}

void handle_while_body_end(int start_label, int end_label) {
    // Gerar código para fim do loop
    emit_while_end_code(start_label, end_label);
    
    // Reinicializar estado de execução
    exec_block = 1;
    current_loop_exit_label = -1;
}

int handle_if_condition(ExpressionResult* condition) {
    // Análise semântica: validar condição
    if (!validate_condition_expression(condition)) {
        return -1; // Erro
    }
    
    // Gerar label
    int else_label = generate_label();
    
    // Atualizar estado de execução
    last_condition_result = evaluate_boolean_expression(condition);
    exec_block = last_condition_result;
    
    // Gerar código
    emit_if_start_code(condition, else_label);
    
    return else_label;
}

// Stack para armazenar labels de fim para statements aninhados
#define MAX_LABEL_STACK 100
static int end_label_stack[MAX_LABEL_STACK];
static int end_label_stack_top = -1;

void push_end_label(int label) {
    if (end_label_stack_top < MAX_LABEL_STACK - 1) {
        end_label_stack[++end_label_stack_top] = label;
    }
}

int pop_end_label() {
    if (end_label_stack_top >= 0) {
        return end_label_stack[end_label_stack_top--];
    }
    return -1;
}

// Variável para armazenar o label de fim atual
static int current_end_label = -1;

void handle_if_else_part(int else_label) {
    // Gerar label único para o fim do if
    int end_label = generate_label();
    push_end_label(end_label);
    
    // Gerar código para else
    emit_if_else_code(else_label, end_label);
    
    // Atualizar estado de execução
    exec_block = !last_condition_result;
}

void handle_if_else_if_part(int else_label) {
    // For else if, we need to set the context flag
    in_else_if_context = 1;
    
    // Gerar label único para o fim do if
    int end_label = generate_label();
    push_end_label(end_label);
    
    // Don't modify exec_block here - it should remain as it is
    // The execution state will be handled by the individual if_stmt
}

int get_current_end_label() {
    return pop_end_label();
}

void handle_if_end_part(int end_label) {
    // Gerar código para fim do if
    emit_if_end_code(end_label);
    
    // Reinicializar estado de execução
    exec_block = 1;
}

// Variável para rastrear se o último argumento de print tinha newline explícito
static int last_print_had_explicit_newline = 0;

// Ações de E/O
void handle_print_statement() {
    // Não adiciona newline automático - todos os newlines devem ser explícitos
    // Isso garante formatação consistente para matrizes
    // Reset para próxima print statement (mesmo que não usado)
    last_print_had_explicit_newline = 0;
}

void handle_read_statement(LValueResult* lval) {
    // Análise semântica: validar lvalue
    if (lval->type == LVALUE_VAR && !find_variable_in_scopes(lval->varName)) {
        semantic_error("Variable '%s' not declared", lval->varName);
        return;
    }
    
    // Gerar código
    emit_read_code(lval);
}

void handle_print_expression(ExpressionResult* expr) {
    // Gerar código para impressão de expressão
    emit_print_code(expr);
    
    // Verificar se a expressão é uma string que contém \n
    if (expr && strcmp(expr->type, "string") == 0 && expr->strVal) {
        if (strstr(expr->strVal, "\\n") != NULL) {
            last_print_had_explicit_newline = 1;
        }
        // Note: não resetamos para 0 aqui, só acumulamos se encontramos \n
    }
}

// Ações de Atribuição
void handle_assignment(LValueResult* lval, ExpressionResult* expr) {
    // Análise semântica: validar atribuição
    const char* target_type;
    
    if (lval->type == LVALUE_VAR) {
        // Atribuição simples a variável
        if (!validate_variable_assignment(lval->varName, expr)) {
            return;
        }
        target_type = expr->type;
    } else if (lval->type == LVALUE_ARRAY_ACCESS) {
        // Atribuição a elemento de array
        if (!check_type_compatibility(lval->elementType, expr->type)) {
            semantic_error("Tipo incompatível: não é possível atribuir %s a elemento de tipo %s", 
                          expr->type, lval->elementType);
            return;
        }
        target_type = lval->elementType;
    } else if (lval->type == LVALUE_STRUCT_FIELD) {
        // Atribuição a campo de struct
        if (!check_type_compatibility(lval->elementType, expr->type)) {
            semantic_error("Tipo incompatível: não é possível atribuir %s a campo de tipo %s", 
                          expr->type, lval->elementType);
            return;
        }
        target_type = lval->elementType;
    } else {
        semantic_error("Tipo de lvalue não suportado para atribuição");
        return;
    }
    
    // Gerar código
    emit_assignment_code(lval, expr);
}

void handle_increment(LValueResult* lval) {
    // Análise semântica: validar incremento
    if (!validate_loop_increment(lval)) {
        return;
    }
    
    // Atualizar tabela de símbolos
    increment_variable(lval->varName);
    
    // Gerar código
    emit_increment_code(lval);
}

void handle_decrement(LValueResult* lval) {
    // Análise semântica: validar decremento
    if (!validate_loop_increment(lval)) {
        return;
    }
    
    // Atualizar tabela de símbolos
    decrement_variable(lval->varName);
    
    // Gerar código
    emit_decrement_code(lval);
}

// Ações de Criação de Expressões
ExpressionResult* create_bool_expression(double value) {
    // Criar resultado da expressão
    ExpressionResult* result = malloc(sizeof(ExpressionResult));
    result->type = strdup("bool");
    result->intVal = (int)value;
    
    // Gerar representação de código
    result->c_code = malloc(16);
    snprintf(result->c_code, 16, "%d", (int)value);
    
    return result;
}

ExpressionResult* create_int_expression(double value) {
    ExpressionResult* result = malloc(sizeof(ExpressionResult));
    result->type = strdup("int");
    result->intVal = (int)value;
    
    result->c_code = malloc(32);
    snprintf(result->c_code, 32, "%d", (int)value);
    
    return result;
}

ExpressionResult* create_float_expression(double value) {
    ExpressionResult* result = malloc(sizeof(ExpressionResult));
    result->type = strdup("float");
    result->doubleVal = value;
    
    result->c_code = malloc(32);
    snprintf(result->c_code, 32, "%f", value);
    
    return result;
}

ExpressionResult* create_string_expression(const char* value) {
    ExpressionResult* result = malloc(sizeof(ExpressionResult));
    result->type = strdup("string");
    result->strVal = strdup(value);
    
    result->c_code = strdup(value);
    
    return result;
}

ExpressionResult* create_lvalue_expression(LValueResult* lval) {
    // Delegar para semântica para avaliação
    if (lval->type == LVALUE_VAR) {
        return evaluate_variable_access(lval->varName);
    } else if (lval->type == LVALUE_ARRAY_ACCESS) {
        // Para acesso a array, gerar código C de acesso e alocar 2D arrays se necessário
        ExpressionResult* result = malloc(sizeof(ExpressionResult));
        if (!result) {
            semantic_error("Erro de alocação de memória para resultado de expressão");
            return NULL;
        }
        
        result->type = strdup(lval->elementType);
        result->strVal = NULL;
        
        // Auto-aloca array 2D se necessário antes de gerar código de acesso
        if (lval->dimensionCount == 2) {
            emit_auto_allocate_2d_array(lval->varName);
        }
        
        // Gera código C para acesso ao array
        char array_access[256] = "";
        snprintf(array_access, sizeof(array_access), "%s", lval->varName);
        
        for (int i = 0; i < lval->dimensionCount; i++) {
            char index_str[64];
            snprintf(index_str, sizeof(index_str), "[%s]", lval->indexExpressions[i]);
            strcat(array_access, index_str);
        }
        
        result->c_code = strdup(array_access);
        
        // Definir valores padrão baseados no tipo
        if (strcmp(lval->elementType, "int") == 0) {
            result->intVal = 0;
        } else if (strcmp(lval->elementType, "float") == 0) {
            result->doubleVal = 0.0;
        } else if (strcmp(lval->elementType, "bool") == 0) {
            result->intVal = 0;
        } else if (strcmp(lval->elementType, "string") == 0) {
            result->strVal = "";
        }
        
        return result;
    } else if (lval->type == LVALUE_STRUCT_FIELD) {
        // Para acesso a campo de struct, gerar código C de acesso
        ExpressionResult* result = malloc(sizeof(ExpressionResult));
        if (!result) {
            semantic_error("Erro de alocação de memória para resultado de expressão");
            return NULL;
        }
        
        result->type = strdup(lval->elementType);
        result->strVal = NULL;
        
        // Gera código C para acesso ao campo: struct_var.field_name
        char field_access[256];
        snprintf(field_access, sizeof(field_access), "%s.%s", lval->varName, lval->fieldName);
        result->c_code = strdup(field_access);
        
        // Definir valores padrão baseados no tipo
        if (strcmp(lval->elementType, "int") == 0) {
            result->intVal = 0;
        } else if (strcmp(lval->elementType, "float") == 0) {
            result->doubleVal = 0.0;
        } else if (strcmp(lval->elementType, "bool") == 0) {
            result->intVal = 0;
        } else if (strcmp(lval->elementType, "string") == 0) {
            result->strVal = "";
        }
        
        return result;
    }
    
    return NULL;
}

// Binary Operation Actions
ExpressionResult* handle_addition(ExpressionResult* left, ExpressionResult* right) {
    // Delegate to semantics for evaluation
    return evaluate_binary_expression(left, right, "+");
}

ExpressionResult* handle_subtraction(ExpressionResult* left, ExpressionResult* right) {
    return evaluate_binary_expression(left, right, "-");
}

ExpressionResult* handle_multiplication(ExpressionResult* left, ExpressionResult* right) {
    return evaluate_binary_expression(left, right, "*");
}

ExpressionResult* handle_division(ExpressionResult* left, ExpressionResult* right) {
    return evaluate_binary_expression(left, right, "/");
}

ExpressionResult* handle_exponentiation(ExpressionResult* left, ExpressionResult* right) {
    return evaluate_binary_expression(left, right, "^");
}

ExpressionResult* handle_less_than(ExpressionResult* left, ExpressionResult* right) {
    return evaluate_binary_expression(left, right, "<");
}

ExpressionResult* handle_greater_than(ExpressionResult* left, ExpressionResult* right) {
    return evaluate_binary_expression(left, right, ">");
}

ExpressionResult* handle_less_equal(ExpressionResult* left, ExpressionResult* right) {
    return evaluate_binary_expression(left, right, "<=");
}

ExpressionResult* handle_greater_equal(ExpressionResult* left, ExpressionResult* right) {
    return evaluate_binary_expression(left, right, ">=");
}

ExpressionResult* handle_equals(ExpressionResult* left, ExpressionResult* right) {
    return evaluate_binary_expression(left, right, "==");
}

ExpressionResult* handle_logical_and(ExpressionResult* left, ExpressionResult* right) {
    return evaluate_binary_expression(left, right, "&&");
}

ExpressionResult* handle_logical_or(ExpressionResult* left, ExpressionResult* right) {
    return evaluate_binary_expression(left, right, "||");
}

ExpressionResult* handle_modulo(ExpressionResult* left, ExpressionResult* right) {
    return evaluate_binary_expression(left, right, "%");
}

ExpressionResult* handle_unary_minus(ExpressionResult* expr) {
    return evaluate_unary_expression(expr, "-");
}

ExpressionResult* handle_logical_not(ExpressionResult* expr) {
    return evaluate_unary_expression(expr, "!");
}

// Function call and array operations
ExpressionResult* handle_function_call(const char* func_name) {
    // Busca o tipo de retorno da função na tabela de símbolos
    char* return_type = get_function_return_type(func_name);
    
    if (!return_type) {
        semantic_error("Função '%s' não foi declarada", func_name);
        return NULL;
    }
    
    ExpressionResult* result = malloc(sizeof(ExpressionResult));
    if (!result) {
        semantic_error("Erro de alocação de memória");
        free(return_type);
        return NULL;
    }
    
    result->type = return_type;  // Usa o tipo de retorno correto
    result->intVal = 0;
    result->strVal = NULL;
    
    // Generate function call code
    char* call_code = malloc(strlen(func_name) + 10);
    if (call_code) {
        sprintf(call_code, "%s()", func_name);
        result->c_code = call_code;
    } else {
        result->c_code = NULL;
    }
    
    return result;
}

// Function call with arguments
ExpressionResult* handle_function_call_with_args(const char* func_name, const char* args) {
    // Busca o tipo de retorno da função na tabela de símbolos
    char* return_type = get_function_return_type(func_name);
    
    if (!return_type) {
        semantic_error("Função '%s' não foi declarada", func_name);
        return NULL;
    }
    
    ExpressionResult* result = malloc(sizeof(ExpressionResult));
    if (!result) {
        semantic_error("Erro de alocação de memória");
        free(return_type);
        return NULL;
    }
    
    result->type = return_type;  // Usa o tipo de retorno correto
    result->intVal = 0;
    result->strVal = NULL;
    
    // Generate function call code with arguments, handling reference parameters
    char* call_code = generate_function_call_with_references(func_name, args);
    result->c_code = call_code;
    
    return result;
}

ExpressionResult* handle_len_expression(ExpressionResult* array_expr) {
    // Validate that expression is an array
    if (!array_expr || !array_expr->type || strstr(array_expr->type, "[]") == NULL) {
        semantic_error("Função len() só pode ser aplicada a arrays");
        return NULL;
    }
    
    ExpressionResult* result = malloc(sizeof(ExpressionResult));
    if (!result) {
        semantic_error("Erro de alocação de memória");
        return NULL;
    }
    
    result->type = strdup("int");
    result->intVal = 5;  // Placeholder - for static arrays, could be computed
    
    // Generate C code for length - for now, use a hardcoded value
    // In a real implementation, this would need to track array sizes
    result->c_code = strdup("5");
    result->strVal = NULL;
    
    return result;
}

ExpressionResult* handle_array_literal(const char* element_type) {
    // Cria expressão de literal de array
    ExpressionResult* result = malloc(sizeof(ExpressionResult));
    if (!result) {
        semantic_error("Erro de alocação de memória");
        return NULL;
    }
    
    // Cria tipo de array
    char* array_type = malloc(strlen(element_type) + 3);
    if (!array_type) {
        semantic_error("Erro de alocação de memória");
        free(result);
        return NULL;
    }
    
    sprintf(array_type, "%s[]", element_type);
    result->type = array_type;
    result->intVal = 0;
    result->c_code = strdup("{/* literal de array */}");
    result->strVal = NULL;
    
    return result;
}

ExpressionResult* handle_array_literal_with_values(const char* values) {
    // Cria expressão de literal de array com valores reais
    ExpressionResult* result = malloc(sizeof(ExpressionResult));
    if (!result) {
        semantic_error("Erro de alocação de memória");
        return NULL;
    }
    
    // Define tipo como int[] (precisaremos inferir isso melhor depois)
    result->type = strdup("int[]");
    result->intVal = 0;
    
    // Generate proper C array initialization
    char* c_code = malloc(strlen(values) + 3);
    if (c_code) {
        sprintf(c_code, "{%s}", values);
        result->c_code = c_code;
    } else {
        result->c_code = strdup("{/* literal de array */}");
    }
    
    result->strVal = NULL;
    
    return result;
}

// Type creation functions
char* create_array_type(const char* base_type) {
    char* array_type = malloc(strlen(base_type) + 3);
    if (!array_type) {
        semantic_error("Erro de alocação de memória");
        return NULL;
    }
    
    sprintf(array_type, "%s[]", base_type);
    return array_type;
}

char* create_2d_array_type(const char* base_type) {
    char* array_type = malloc(strlen(base_type) + 5);
    if (!array_type) {
        semantic_error("Erro de alocação de memória");
        return NULL;
    }
    
    sprintf(array_type, "%s[][]", base_type);
    return array_type;
}

char* create_3d_array_type(const char* base_type) {
    char* array_type = malloc(strlen(base_type) + 7);
    if (!array_type) {
        semantic_error("Erro de alocação de memória");
        return NULL;
    }
    
    sprintf(array_type, "%s[][][]", base_type);
    return array_type;
}

// Scope management functions
void handle_function_start(const char* return_type, const char* func_name) {
    // Push function scope
    push_scope(func_name);
    
    // Generate function code
    if (generate_code) {
        emit_line("%s %s() {", convert_penelope_type_to_c(return_type), func_name);
        increase_indent();
    }
}

void handle_function_end() {
    // Pop function scope
    pop_scope();
    
    // Generate function end code
    if (generate_code) {
        decrease_indent();
        emit_line("}");
    }
}

void handle_block_start() {
    // Push block scope
    push_scope("block");
    
    // Generate block code
    if (generate_code) {
        emit_line("{");
        increase_indent();
    }
}

void handle_block_end() {
    // Pop block scope
    pop_scope();
    
    // Generate block end code
    if (generate_code) {
        decrease_indent();
        emit_line("}");
    }
}

void handle_for_start() {
    // Push for scope
    push_scope("for");
}

void handle_for_end() {
    // Pop for scope
    pop_scope();
}

// Parameter handling
void handle_parameter_declaration(const char* type, const char* param_name) {
    // Declare parameter in current scope
    declare_variable(type, param_name, NULL);
}

// Function parameter handling
void handle_function_parameter(const char* type, const char* param_name) {
    // Isso será tratado coletando parâmetros e emitindo-os
    // in the function signature. For now, just add to parameter list.
    // TODO: Implementar coleta e emissão de parâmetros
}

// Break statement
void handle_break_statement() {
    // Gerar código para break
    if (generate_code && current_loop_exit_label != -1) {
        emit_line("goto L%d;", current_loop_exit_label);
    } else {
        semantic_error("Break statement não pode ser usado fora de loop");
    }
}

// Return statement handling
void handle_return_statement(ExpressionResult* expr) {
    if (!generate_code) return;
    
    if (expr && expr->c_code) {
        emit_line("return %s;", expr->c_code);
    } else if (expr) {
        if (strcmp(expr->type, "int") == 0) {
            emit_line("return %d;", expr->intVal);
        } else if (strcmp(expr->type, "float") == 0) {
            emit_line("return %f;", expr->doubleVal);
        } else if (strcmp(expr->type, "bool") == 0) {
            emit_line("return %d;", expr->intVal);
        } else if (strcmp(expr->type, "string") == 0) {
            emit_line("return %s;", expr->strVal ? expr->strVal : "\"\"");
        }
    } else {
        emit_line("return;");
    }
}

// Function signature storage
typedef struct FunctionSignature {
    char* name;
    char* return_type;
    char** param_types;
    int param_count;
} FunctionSignature;

// Global storage for function signatures
#define MAX_FUNCTIONS 100
FunctionSignature function_signatures[MAX_FUNCTIONS];
int function_count = 0;

// Function to store function signature
void store_function_signature(const char* name, const char* return_type, FunctionParam* params) {
    if (function_count >= MAX_FUNCTIONS) return;
    
    FunctionSignature* sig = &function_signatures[function_count];
    sig->name = strdup(name);
    sig->return_type = strdup(return_type);
    
    // Count parameters
    int count = 0;
    FunctionParam* curr = params;
    while (curr) {
        count++;
        curr = curr->next;
    }
    
    sig->param_count = count;
    sig->param_types = malloc(count * sizeof(char*));
    
    // Store parameter types in reverse order since they're added to front of list
    char** param_array = malloc(count * sizeof(char*));
    curr = params;
    for (int i = count - 1; i >= 0; i--) {
        param_array[i] = strdup(curr->type);
        curr = curr->next;
    }
    
    sig->param_types = param_array;
    
    function_count++;
}

// Function to get function signature
FunctionSignature* get_function_signature(const char* name) {
    for (int i = 0; i < function_count; i++) {
        if (strcmp(function_signatures[i].name, name) == 0) {
            return &function_signatures[i];
        }
    }
    return NULL;
}

// Function to generate function call with proper reference handling
char* generate_function_call_with_references(const char* func_name, const char* args) {
    FunctionSignature* sig = get_function_signature(func_name);
    if (!sig) {
        // Fallback to simple function call
        char* call_code = malloc(strlen(func_name) + strlen(args) + 10);
        if (call_code) {
            sprintf(call_code, "%s(%s)", func_name, args);
        }
        return call_code;
    } else {
        // Parse arguments and modify reference parameters
        char* modified_args = strdup(args);
        char* result_call = malloc(strlen(func_name) + strlen(modified_args) + 10);
        
        // For now, use simple approach - this would need more sophisticated argument parsing
        // to handle multiple parameters correctly
        if (sig->param_count > 0) {
            // Check if we have reference parameters and modify accordingly
            // This is a simplified implementation - full implementation would need proper argument parsing
            
            // Handle single parameter functions (like test)
            if (sig->param_count == 1) {
                // Check if the parameter is a reference
                if (sig->param_types[0] && strstr(sig->param_types[0], "&")) {
                    // Check if the argument is already a pointer parameter
                    // Trim whitespace from args
                    char* trimmed_args = strdup(args);
                    char* start = trimmed_args;
                    while (*start == ' ') start++;
                    
                    Data* arg_data = find_variable_in_scopes(start);
                    int arg_is_reference_param = 0;
                    if (arg_data && arg_data->type) {
                        int type_len = strlen(arg_data->type);
                        if (type_len > 0 && arg_data->type[type_len - 1] == '&') {
                            arg_is_reference_param = 1;
                        }
                    }
                    
                    if (arg_is_reference_param) {
                        // Don't add & for reference parameters
                        sprintf(result_call, "%s(%s)", func_name, start);
                    } else {
                        // Add & for regular variables
                        sprintf(result_call, "%s(&%s)", func_name, start);
                    }
                    free(trimmed_args);
                } else {
                    sprintf(result_call, "%s(%s)", func_name, args);
                }
            }
            // Handle three parameter functions (like mdc)
            else if (sig->param_count == 3) {
                // Parse the arguments to find the third one
                char* args_copy = strdup(args);
                char* token1 = strtok(args_copy, ",");
                char* token2 = strtok(NULL, ",");
                char* token3 = strtok(NULL, ",");
                
                if (token1 && token2 && token3) {
                    // Trim whitespace from token3
                    while (*token3 == ' ') token3++;
                    
                    // Check if third parameter is a reference
                    if (sig->param_types[2] && strstr(sig->param_types[2], "&")) {
                        // Check if the argument is already a pointer parameter
                        // If the argument is a function parameter with reference type, don't add &
                        Data* arg_data = find_variable_in_scopes(token3);
                        int arg_is_reference_param = 0;
                        if (arg_data && arg_data->type) {
                            int type_len = strlen(arg_data->type);
                            if (type_len > 0 && arg_data->type[type_len - 1] == '&') {
                                arg_is_reference_param = 1;
                            }
                        }
                        
                        if (arg_is_reference_param) {
                            // Don't add & for reference parameters
                            sprintf(result_call, "%s(%s, %s, %s)", func_name, token1, token2, token3);
                        } else {
                            // Add & for regular variables
                            sprintf(result_call, "%s(%s, %s, &%s)", func_name, token1, token2, token3);
                        }
                    } else {
                        sprintf(result_call, "%s(%s)", func_name, args);
                    }
                } else {
                    sprintf(result_call, "%s(%s)", func_name, args);
                }
                free(args_copy);
            } else {
                sprintf(result_call, "%s(%s)", func_name, args);
            }
        } else {
            sprintf(result_call, "%s(%s)", func_name, args);
        }
        
        free(modified_args);
        return result_call;
    }
}
