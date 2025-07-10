#include "semantics.h"
#include "../symbolTable/symbolTable.h"
#include "../codeGenerator/codeGenerator.h"
#include "../expressionUtils/expressionUtils.h"
#include "../../structs/lvalue/lvalueResult.h"
#include "../../structs/expression/expressionResult.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

// Declaração antecipada
Data* find_variable(const char* var_name);

// Variáveis globais para controle semântico
int semantic_errors = 0;
int syntax_errors = 0;
int exec_block = 1;
int last_condition_result = 0;

// Flags para controle de formatação de impressão
int is_matrix_element_print = 0;
int has_explicit_newline = 0;
int last_print_arg_count = 0;

// Função global para reportar erros semânticos com linha
extern int yylineno;

void semantic_error(const char* format, ...) {
    semantic_errors++;
    fprintf(stderr, "Erro Semântico na linha %d: ", yylineno);
    
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);  // Imprime a mensagem formatada
    va_end(args);
    
    fprintf(stderr, "\n");
}

void print_string(char *str) {
    // Não gera saída em tempo de execução - apenas gera código C
    if (str && str[0] == '"' && str[strlen(str)-1] == '"') {
        printf("%s ", str + 1);   // Pula aspas iniciais, adiciona espaço
    } else {
        printf("%s ", str);
    }
}

void print_newline() {
    // Função para controle de quebra de linha em impressões
    has_explicit_newline = 1;
    if (generate_code) {
        emit_line("printf(\"\\n\");");
    }
}

double power_operation(double base, double exponent) {
    return pow(base, exponent);
}

// Funções de análise semântica para gerenciamento de variáveis
int validate_variable_declaration(const char* type, const char* var_name) {
    if (find_variable_in_current_scope(var_name) != NULL) {
        semantic_error("Variável '%s' já declarada no escopo atual.", var_name);
        return 0;
    }
    return 1;
}

int validate_variable_assignment(const char* var_name, ExpressionResult* expr) {
    Data* var_data = find_variable(var_name);
    if (var_data == NULL) {
        semantic_error("Variável '%s' não declarada.", var_name);
        return 0;
    }
    
    if (!check_type_compatibility(var_data->type, expr->type)) {
        semantic_error("Tipo incompatível: variável de tipo %s não pode receber tipo %s", 
                      var_data->type, expr->type);
        return 0;
    }
    
    return 1;
}

int validate_array_access(const char* var_name, ExpressionResult* index) {
    Data* var_data = find_variable(var_name);
    if (var_data == NULL) {
        semantic_error("Variável '%s' não declarada.", var_name);
        return 0;
    }
    
    if (strstr(var_data->type, "[]") == NULL) {
        semantic_error("Variável '%s' não é um array.", var_name);
        return 0;
    }
    
    if (strcmp(index->type, "int") != 0) {
        semantic_error("Índice do array deve ser do tipo int.");
        return 0;
    }
    
    return 1;
}

int validate_binary_operation(ExpressionResult* left, ExpressionResult* right, const char* operation) {
    if (strcmp(operation, "+") == 0 || strcmp(operation, "-") == 0 || 
    strcmp(operation, "*") == 0 || strcmp(operation, "/") == 0 ||
    strcmp(operation, "^") == 0 || strcmp(operation, "%") == 0) {
    
      if ((strcmp(left->type, "int") == 0 || strcmp(left->type, "float") == 0) &&
        (strcmp(right->type, "int") == 0 || strcmp(right->type, "float") == 0)) {

        // Regra especial: módulo só faz sentido para inteiros
        if (strcmp(operation, "%") == 0 &&
            (strcmp(left->type, "int") != 0 || strcmp(right->type, "int") != 0)) {
            semantic_error("Operação %% requer operandos do tipo int");
            return 0;
        }

        return 1;
    }

    // Concatenação de strings
    if (strcmp(operation, "+") == 0 && strcmp(left->type, "string") == 0 && strcmp(right->type, "string") == 0) {
        return 1;
    }

    semantic_error("Operação %s não suportada entre tipos %s e %s", operation, left->type, right->type);
    return 0;
}
    
    // Operações relacionais
    if (strcmp(operation, "<") == 0 || strcmp(operation, ">") == 0 || 
        strcmp(operation, "<=") == 0 || strcmp(operation, ">=") == 0 ||
        strcmp(operation, "==") == 0 || strcmp(operation, "!=") == 0) {
        
        if (strcmp(left->type, right->type) == 0) {
            return 1;
        }
        
        if ((strcmp(left->type, "int") == 0 || strcmp(left->type, "float") == 0) &&
            (strcmp(right->type, "int") == 0 || strcmp(right->type, "float") == 0)) {
            return 1;
        }
        
        semantic_error("Operação de comparação %s não suportada entre tipos %s e %s", 
                      operation, left->type, right->type);
        return 0;
    }
    
    // Operações lógicas
    if (strcmp(operation, "&&") == 0 || strcmp(operation, "||") == 0) {
        if (strcmp(left->type, "bool") == 0 && strcmp(right->type, "bool") == 0) {
            return 1;
        }
        
        semantic_error("Operação lógica %s requer operandos do tipo bool", operation);
        return 0;
    }
    
    semantic_error("Operação %s não reconhecida", operation);
    return 0;
}

int validate_unary_operation(ExpressionResult* expr, const char* operation) {
    if (strcmp(operation, "-") == 0) {
        if (strcmp(expr->type, "int") == 0 || strcmp(expr->type, "float") == 0) {
            return 1;
        }
        semantic_error("Operação unária - não suportada para tipo %s", expr->type);
        return 0;
    }
    
    if (strcmp(operation, "!") == 0) {
        if (strcmp(expr->type, "bool") == 0) {
            return 1;
        }
        semantic_error("Operação unária ! requer operando do tipo bool");
        return 0;
    }
    
    semantic_error("Operação unária %s não reconhecida", operation);
    return 0;
}

// Funções de análise semântica para verificação de tipos
int check_type_compatibility(const char* expected_type, const char* actual_type) {
    if (strcmp(expected_type, actual_type) == 0) {
        return 1;
    }
    
    // Compatibilidade entre int e float
if ((strcmp(expected_type, "float") == 0 && strcmp(actual_type, "int") == 0) || (strcmp(expected_type, "int") == 0 && strcmp(actual_type, "float") == 0)) {
        return 1;
    }

    
    return 0;
}

char* get_expression_result_type(ExpressionResult* expr) {
    if (expr && expr->type) {
        return strdup(expr->type);
    }
    return strdup("unknown");
}

char* get_binary_operation_result_type(const char* left_type, const char* right_type, const char* operation) {
    // Operações aritméticas
if (strcmp(operation, "+") == 0 || strcmp(operation, "-") == 0 || 
    strcmp(operation, "*") == 0 || strcmp(operation, "/") == 0 ||
    strcmp(operation, "^") == 0 || strcmp(operation, "%") == 0) {
    
    if (strcmp(left_type, "float") == 0 || strcmp(right_type, "float") == 0) {
        return strdup("float");
    }
    if (strcmp(left_type, "int") == 0 && strcmp(right_type, "int") == 0) {
        return strdup("int");
    }
    if (strcmp(operation, "+") == 0 && strcmp(left_type, "string") == 0 && strcmp(right_type, "string") == 0) {
        return strdup("string");
    }
}
    
    // Operações relacionais
    if (strcmp(operation, "<") == 0 || strcmp(operation, ">") == 0 || 
        strcmp(operation, "<=") == 0 || strcmp(operation, ">=") == 0 ||
        strcmp(operation, "==") == 0 || strcmp(operation, "!=") == 0) {
        return strdup("bool");
    }
    
    // Operações lógicas
    if (strcmp(operation, "&&") == 0 || strcmp(operation, "||") == 0) {
        return strdup("bool");
    }
    
    return strdup("unknown");
}

// Funções de análise semântica para controle de fluxo
int validate_condition_expression(ExpressionResult* condition) {
    if (condition == NULL) {
        semantic_error("Condição não pode ser nula");
        return 0;
    }
    
    if (strcmp(condition->type, "bool") == 0) {
        return 1;
    }
    
    // Aceita int como condição (0 = false, != 0 = true)
    if (strcmp(condition->type, "int") == 0) {
        return 1;
    }
    
    semantic_error("Condição deve ser do tipo bool ou int");
    return 0;
}

int validate_loop_increment(LValueResult* lval) {
    if (lval == NULL) {
        semantic_error("LValue não pode ser nulo em incremento");
        return 0;
    }
    
    Data* var_data = find_variable(lval->varName);
    if (var_data == NULL) {
        semantic_error("Variável '%s' não declarada", lval->varName);
        return 0;
    }
    
    if (strcmp(var_data->type, "int") != 0 && strcmp(var_data->type, "float") != 0) {
        semantic_error("Incremento/decremento só suportado para tipos int e float");
        return 0;
    }
    
    return 1;
}

// Operações semânticas da tabela de símbolos
int declare_variable(const char* type, const char* var_name, ExpressionResult* initial_value) {
    char *fullKey = malloc(strlen(currentScope) + strlen(var_name) + 2);
    if (!fullKey) {
        semantic_error("Erro de alocação de memória para chave do símbolo.");
        return 0;
    }
    
    sprintf(fullKey, "%s#%s", currentScope, var_name);

    Data data;
    data.type = strdup(type);
    
    // Inicializa todos os membros da união com valores seguros
    data.value.intVal = 0;
    data.value.doubleVal = 0.0;
    data.value.strVal = NULL;
    
    if (initial_value != NULL) {
        if (strcmp(type, "int") == 0) {
            data.value.intVal = initial_value->intVal;
        } else if (strcmp(type, "float") == 0) {
            data.value.doubleVal = initial_value->doubleVal;
        } else if (strcmp(type, "bool") == 0) {
            data.value.intVal = initial_value->intVal;
        } else if (strcmp(type, "string") == 0) {
            data.value.strVal = strdup(initial_value->strVal);
        }
    }

    insert_node(&symbolTable, fullKey, data);
    free(fullKey);
    return 1;
}

int assign_to_variable(const char* var_name, ExpressionResult* value) {
    Data* var_data = find_variable(var_name);
    if (var_data == NULL) {
        semantic_error("Variável '%s' não declarada", var_name);
        return 0;
    }
    
    if (!check_type_compatibility(var_data->type, value->type)) {
        semantic_error("Tipo incompatível na atribuição à variável '%s'", var_name);
        return 0;
    }
    
    // Atualiza valor na tabela de símbolos
    if (strcmp(var_data->type, "int") == 0) {
        var_data->value.intVal = value->intVal;
    } else if (strcmp(var_data->type, "float") == 0) {
        var_data->value.doubleVal = value->doubleVal;
    } else if (strcmp(var_data->type, "bool") == 0) {
        var_data->value.intVal = value->intVal;
    } else if (strcmp(var_data->type, "string") == 0) {
        // Libera o valor antigo da string para evitar vazamento de memória
        if (var_data->value.strVal) {
            free(var_data->value.strVal);
        }
        var_data->value.strVal = strdup(value->strVal);
    }
    
    return 1;
}

int increment_variable(const char* var_name) {
    Data* var_data = find_variable(var_name);
    if (var_data == NULL) {
        semantic_error("Variável '%s' não declarada", var_name);
        return 0;
    }
    
    if (strcmp(var_data->type, "int") == 0) {
        var_data->value.intVal++;
    } else if (strcmp(var_data->type, "float") == 0) {
        var_data->value.doubleVal += 1.0;
    } else {
        semantic_error("Incremento não suportado para tipo %s", var_data->type);
        return 0;
    }
    
    return 1;
}

int decrement_variable(const char* var_name) {
    Data* var_data = find_variable(var_name);
    if (var_data == NULL) {
        semantic_error("Variável '%s' não declarada", var_name);
        return 0;
    }
    
    if (strcmp(var_data->type, "int") == 0) {
        var_data->value.intVal--;
    } else if (strcmp(var_data->type, "float") == 0) {
        var_data->value.doubleVal -= 1.0;
    } else {
        semantic_error("Decremento não suportado para tipo %s", var_data->type);
        return 0;
    }
    
    return 1;
}

// Funções de avaliação de expressões
ExpressionResult* evaluate_binary_expression(ExpressionResult* left, ExpressionResult* right, const char* operation) {
    if (!validate_binary_operation(left, right, operation)) {
        return NULL;
    }
    
    ExpressionResult* result = malloc(sizeof(ExpressionResult));
    if (!result) {
        semantic_error("Erro de alocação de memória para resultado de expressão");
        return NULL;
    }
    
    char* result_type = get_binary_operation_result_type(left->type, right->type, operation);
    result->type = result_type;
    result->strVal = NULL;
    
    // Gera código C para a operação binária
    char* left_code = left->c_code ? left->c_code : expression_to_c_code(left);
    char* right_code = right->c_code ? right->c_code : expression_to_c_code(right);
    
    // Tratamento especial para exponentiation
    if (strcmp(operation, "^") == 0) {
        // Para expoentes pequenos e inteiros, usa multiplicação inline
        if (right->type && strcmp(right->type, "int") == 0 && 
            !right->c_code && right->intVal >= 0 && right->intVal <= 3) {
            
            int code_len = strlen(left_code) * right->intVal + 50;
            result->c_code = malloc(code_len);
            
            if (right->intVal == 0) {
                snprintf(result->c_code, code_len, "1");
            } else if (right->intVal == 1) {
                snprintf(result->c_code, code_len, "(%s)", left_code);
            } else if (right->intVal == 2) {
                snprintf(result->c_code, code_len, "((%s) * (%s))", left_code, left_code);
            } else if (right->intVal == 3) {
                snprintf(result->c_code, code_len, "((%s) * (%s) * (%s))", left_code, left_code, left_code);
            }
        } else {
            // Para casos complexos, usa pow() da math.h
            int code_len = strlen(left_code) + strlen(right_code) + 20;
            result->c_code = malloc(code_len);
            snprintf(result->c_code, code_len, "pow(%s, %s)", left_code, right_code);
        }
    } else {
        // Operações normais (numéricas, comparações, etc.)
        int code_len = strlen(left_code) + strlen(right_code) + strlen(operation) + 10;
        result->c_code = malloc(code_len);
        snprintf(result->c_code, code_len, "(%s %s %s)", left_code, operation, right_code);
    }
    
    // Avaliação das operações (apenas para constantes)
    if (strcmp(operation, "+") == 0) {
        if (strcmp(result_type, "int") == 0) {
            result->intVal = left->intVal + right->intVal;
        } else if (strcmp(result_type, "float") == 0) {
            result->doubleVal = (strcmp(left->type, "float") == 0 ? left->doubleVal : left->intVal) + 
                               (strcmp(right->type, "float") == 0 ? right->doubleVal : right->intVal);
        } else if (strcmp(result_type, "string") == 0) {
            // Concatenação de strings
            int len = strlen(left->strVal) + strlen(right->strVal) + 1;
            result->strVal = malloc(len);
            if (result->strVal) {
                strcpy(result->strVal, left->strVal);
                strcat(result->strVal, right->strVal);
            }
        }
    }
    // Outras operações seriam implementadas similarmente...
    
    return result;
}

ExpressionResult* evaluate_unary_expression(ExpressionResult* expr, const char* operation) {
    if (!validate_unary_operation(expr, operation)) {
        return NULL;
    }
    
    ExpressionResult* result = malloc(sizeof(ExpressionResult));
    if (!result) {
        semantic_error("Erro de alocação de memória para resultado de expressão");
        return NULL;
    }
    
    result->type = strdup(expr->type);
    result->c_code = NULL;
    result->strVal = NULL;
    
    if (strcmp(operation, "-") == 0) {
        if (strcmp(expr->type, "int") == 0) {
            result->intVal = -expr->intVal;
        } else if (strcmp(expr->type, "float") == 0) {
            result->doubleVal = -expr->doubleVal;
        }
    } else if (strcmp(operation, "!") == 0) {
        result->intVal = !expr->intVal;
    }
    
    return result;
}

ExpressionResult* evaluate_variable_access(const char* var_name) {
    Data* var_data = find_variable(var_name);
    if (var_data == NULL) {
        semantic_error("Variável '%s' não declarada", var_name);
        return NULL;
    }
    
    ExpressionResult* result = malloc(sizeof(ExpressionResult));
    if (!result) {
        semantic_error("Erro de alocação de memória para resultado de expressão");
        return NULL;
    }
    
    result->type = strdup(var_data->type);
    result->c_code = strdup(var_name);  // C code é simplesmente o nome da variável
    result->strVal = NULL;
    
    if (strcmp(var_data->type, "int") == 0) {
        result->intVal = var_data->value.intVal;
    } else if (strcmp(var_data->type, "float") == 0) {
        result->doubleVal = var_data->value.doubleVal;
    } else if (strcmp(var_data->type, "bool") == 0) {
        result->intVal = var_data->value.intVal;
    } else if (strcmp(var_data->type, "string") == 0) {
        result->strVal = strdup(var_data->value.strVal ? var_data->value.strVal : "");
    }
    
    return result;
}

ExpressionResult* evaluate_array_access(const char* var_name, ExpressionResult* index) {
    if (!validate_array_access(var_name, index)) {
        return NULL;
    }
    
    Data* var_data = find_variable(var_name);
    ExpressionResult* result = malloc(sizeof(ExpressionResult));
    if (!result) {
        semantic_error("Erro de alocação de memória para resultado de expressão");
        return NULL;
    }
    
    // Determina o tipo do elemento do array
    char* element_type = get_array_element_type(var_data->type);
    result->type = element_type;
    result->c_code = NULL;
    result->strVal = NULL;
    
    // Para arrays, retorna valor padrão baseado no tipo
    if (strcmp(element_type, "int") == 0) {
        result->intVal = 0;
    } else if (strcmp(element_type, "float") == 0) {
        result->doubleVal = 0.0;
    } else if (strcmp(element_type, "bool") == 0) {
        result->intVal = 0;
    } else if (strcmp(element_type, "string") == 0) {
        result->strVal = "";
    }
    
    return result;
}

// Funções auxiliares para avaliação de expressões
int evaluate_boolean_expression(ExpressionResult* expr) {
    if (expr == NULL) {
        return 0;
    }
    
    if (strcmp(expr->type, "bool") == 0) {
        return expr->intVal;
    }
    
    if (strcmp(expr->type, "int") == 0) {
        return expr->intVal != 0;
    }
    
    if (strcmp(expr->type, "float") == 0) {
        return expr->doubleVal != 0.0;
    }
    
    return 0;
}

// Função wrapper para find_variable que verifica escopos atuais e pais
Data* find_variable(const char* var_name) {
    Data* result = find_variable_in_scopes(var_name);
    return result;
}
