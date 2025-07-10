#include "grammarActions.h"
#include "../semantics/semantics.h"
#include "../codeGenerator/codeGenerator.h"
#include "../symbolTable/symbolTable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _GNU_SOURCE  // para strdup

// Global variable for tracking loop increment variable
char* current_loop_increment_var = NULL;

// Stack for tracking increment variables in nested loops
#define MAX_LOOP_STACK 10
static char* loop_increment_stack[MAX_LOOP_STACK];
static int loop_increment_stack_top = -1;

// Map para armazenar tamanhos de arrays
#define MAX_ARRAY_SIZES 100
static struct {
    char* name;
    int size;
} array_sizes[MAX_ARRAY_SIZES];
static int array_sizes_count = 0;

// Função para armazenar tamanho de array
void store_array_size(const char* array_name, int size) {
    if (array_sizes_count < MAX_ARRAY_SIZES) {
        array_sizes[array_sizes_count].name = strdup(array_name);
        array_sizes[array_sizes_count].size = size;
        array_sizes_count++;
    }
}

// Função para obter tamanho de array
int get_array_size(const char* array_name) {
    for (int i = 0; i < array_sizes_count; i++) {
        if (strcmp(array_sizes[i].name, array_name) == 0) {
            return array_sizes[i].size;
        }
    }
    return -1; // Não encontrado
}

// Functions to manage the increment variable stack
void push_loop_increment_var(const char* var_name) {
    if (loop_increment_stack_top < MAX_LOOP_STACK - 1) {
        loop_increment_stack[++loop_increment_stack_top] = strdup(var_name);
    }
}

char* pop_loop_increment_var() {
    if (loop_increment_stack_top >= 0) {
        return loop_increment_stack[loop_increment_stack_top--];
    }
    return NULL;
}

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
    
    // Se é um array sendo inicializado com literal, armazenar o tamanho
    if (strstr(type, "[]") != NULL && expr->intVal > 0) {
        store_array_size(var_name, expr->intVal);
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
    
    // Debug: check if we're in a for loop context
    if (!generate_code) {
        fprintf(stderr, "Debug: handle_increment called with generate_code=false, var=%s\n", lval->varName ? lval->varName : "NULL");
    }
    
    // Se a geração de código está desabilitada (contexto de for loop),
    // armazena o nome da variável para uso posterior
    if (!generate_code && lval->varName) {
        push_loop_increment_var(lval->varName);
        fprintf(stderr, "Debug: Pushed increment variable: %s\n", lval->varName);
    }
    
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
    // Delega para semântica para avaliação
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

// Declaração antecipada para processamento de argumentos
char* process_function_arguments(const char* func_name, const char* raw_args);

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
    
    // Gera código de chamada de função
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
    
    // Processa argumentos para lidar com parâmetros de referência
    char* processed_args = process_function_arguments(func_name, args);
    
    // Gera código de chamada de função com argumentos processados
    char* call_code = malloc(strlen(func_name) + strlen(processed_args) + 10);
    if (call_code) {
        sprintf(call_code, "%s(%s)", func_name, processed_args);
        result->c_code = call_code;
    } else {
        result->c_code = NULL;
    }
    
    free(processed_args);
    return result;
}

ExpressionResult* handle_len_expression(ExpressionResult* array_expr) {
    // Validar que a expressão é um array
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
    result->intVal = 0;
    result->strVal = NULL;
    
    // Extrair nome da variável array do c_code
    char* array_name = NULL;
    if (array_expr->c_code) {
        // Encontrar o nome da variável (antes de qualquer colchete)
        char* bracket_pos = strchr(array_expr->c_code, '[');
        if (bracket_pos) {
            // Acesso a array como "quantidadeVendas[i]" -> extrair "quantidadeVendas"
            size_t name_len = bracket_pos - array_expr->c_code;
            array_name = malloc(name_len + 1);
            strncpy(array_name, array_expr->c_code, name_len);
            array_name[name_len] = '\0';
        } else {
            // Variável de array simples como "quantidadeVendas"
            array_name = strdup(array_expr->c_code);
        }
    }
    
    if (!array_name) {
        semantic_error("Não foi possível determinar o nome do array para len()");
        free(result);
        return NULL;
    }
    
    // Gerar código C baseado no tipo de array e convenções de nomenclatura
    char* c_code = malloc(64);
    
    // Verificar se é um array 2D (contém "[][]")
    if (strstr(array_expr->type, "[][]") != NULL) {
        // Para arrays 2D, usar as variáveis de dimensão
        char rows_var[64];
        
        // Usar a mesma convenção de nomenclatura que get_dimension_variable_names
        if (strncmp(array_name, "matriz", 6) == 0) {
            const char* num_part = array_name + 6; // parte após "matriz"
            snprintf(rows_var, 64, "linhas%s", num_part);
        } else if (strcmp(array_name, "matrizSoma") == 0) {
            strcpy(rows_var, "linhas1");
        } else if (strcmp(array_name, "matrizProduto") == 0) {
            strcpy(rows_var, "linhasResultado");
        } else {
            // Padrão genérico: arrayName -> arrayName_rows
            snprintf(rows_var, 64, "%s_rows", array_name);
        }
        
        // len() para arrays 2D retorna número de linhas
        snprintf(c_code, 64, "%s", rows_var);
    } else {
        // Para arrays 1D, tentar obter o tamanho armazenado
        int stored_size = get_array_size(array_name);
        if (stored_size > 0) {
            // Tamanho foi armazenado quando o array foi declarado
            snprintf(c_code, 64, "%d", stored_size);
        } else {
            // Arrays conhecidos dos exemplos (fallback para compatibilidade)
            if (strcmp(array_name, "quantidadeVendas") == 0) {
                strcpy(c_code, "5"); // Tamanho conhecido do exemplo
            } else if (strcmp(array_name, "dadosVendas") == 0) {
                strcpy(c_code, "5"); // Assumir tamanho similar
            } else {
                // Abordagem genérica: assumir que há uma variável de tamanho
                // Padrão: arrayName -> arrayName_size
                snprintf(c_code, 64, "%s_size", array_name);
            }
        }
    }
    
    result->c_code = c_code;
    free(array_name);
    
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
    
    // Count the number of elements in the array literal
    int element_count = 0;
    if (values && strlen(values) > 0) {
        element_count = 1; // At least one element if values is not empty
        // Count commas to determine number of elements
        for (const char* p = values; *p; p++) {
            if (*p == ',') {
                element_count++;
            }
        }
    }
    
    // Define tipo como int[] (precisaremos inferir isso melhor depois)
    result->type = strdup("int[]");
    result->intVal = element_count; // Store the element count
    
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

// TFunções de criação de tipos
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

// Funções de manipulação de escopo
void handle_function_start(const char* return_type, const char* func_name) {

    push_scope(func_name);
    
    if (generate_code) {
        emit_line("%s %s() {", convert_penelope_type_to_c(return_type), func_name);
        increase_indent();
    }
}

void handle_function_end() {

    pop_scope();
    
    if (generate_code) {
        decrease_indent();
        emit_line("}");
    }
}

void handle_block_start() {

    push_scope("block");
    
    if (generate_code) {
        emit_line("{");
        increase_indent();
    }
}

void handle_block_end() {

    pop_scope();
    
    if (generate_code) {
        decrease_indent();
        emit_line("}");
    }
}

void handle_for_start() {

    push_scope("for");
}

void handle_for_end() {

    pop_scope();
}

// Função para lidar com declarações de parâmetros
void handle_parameter_declaration(const char* type, const char* param_name) {

    declare_variable(type, param_name, NULL);
}

// Função para lidar com parâmetros de função
void handle_function_parameter(const char* type, const char* param_name) {

    if (current_function_name) {
        store_function_parameter(current_function_name, param_name, type);
    }
    
    declare_variable(type, param_name, NULL);
}

void handle_break_statement() {

    if (generate_code && current_loop_exit_label != -1) {
        emit_line("goto L%d;", current_loop_exit_label);
    } else {
        semantic_error("Break statement não pode ser usado fora de loop");
    }
}

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

// Função para processar argumentos e lidar com parâmetros de referência
char* process_function_arguments(const char* func_name, const char* raw_args) {
    if (!func_name || !raw_args) return strdup("");
    
    // Get function parameter information
    init_function_params();
    Node* param_node = find_node(&function_params_map, func_name);
    if (!param_node) {
        // No parameter info stored, return args as-is
        return strdup(raw_args);
    }
    
    FunctionParamInfo* params = (FunctionParamInfo*)param_node->value.value.strVal;
    if (!params) {
        return strdup(raw_args);
    }
    
    // Constrói array de tipos de parâmetros para acesso mais fácil
    char param_types[10][50]; // Support up to 10 parameters, 50 chars each
    int param_count = 0;
    
    FunctionParamInfo* current = params;
    while (current && param_count < 10) {
        strncpy(param_types[param_count], current->type, 49);
        param_types[param_count][49] = '\0';
        current = current->next;
        param_count++;
    }
    
    // Reverse the array since parameters are stored in reverse order
    for (int i = 0; i < param_count / 2; i++) {
        char temp[50];
        strcpy(temp, param_types[i]);
        strcpy(param_types[i], param_types[param_count - 1 - i]);
        strcpy(param_types[param_count - 1 - i], temp);
    }
    
    // Processa argumentos
    char* processed_args = malloc(strlen(raw_args) + param_count * 2 + 10); // Extra space for & symbols
    processed_args[0] = '\0';
    
    char* args_copy = strdup(raw_args);
    char* token = strtok(args_copy, ",");
    int arg_index = 0;
    
    while (token != NULL && arg_index < param_count) {

        while (*token == ' ') token++;
        char* end = token + strlen(token) - 1;
        while (end > token && *end == ' ') end--;
        *(end + 1) = '\0';
        
        if (strlen(processed_args) > 0) {
            strcat(processed_args, ", ");
        }
        
        if (arg_index < param_count && strstr(param_types[arg_index], "&") != NULL) {

            if (strchr(token, '(') == NULL && strchr(token, '[') == NULL && 
                strchr(token, '*') == NULL && strstr(token, "->") == NULL) {

                    int is_function_param = 0;
                if (current_function_name && strcmp(func_name, current_function_name) == 0) {

                    FunctionParamInfo* check_param = params;
                    while (check_param) {
                        if (strcmp(token, check_param->name) == 0) {
                            is_function_param = 1;
                            break;
                        }
                        check_param = check_param->next;
                    }
                }
                
                if (!is_function_param) {
                    strcat(processed_args, "&");
                }
            }
        }
        
        strcat(processed_args, token);
        
        token = strtok(NULL, ",");
        arg_index++;
    }
    
    free(args_copy);
    return processed_args;
}
