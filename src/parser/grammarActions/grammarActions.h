#ifndef GRAMMAR_ACTIONS_H
#define GRAMMAR_ACTIONS_H

#include "../symbolTable/symbolTable.h"
#include "../../structs/expression/expressionResult.h"
#include "../../structs/lvalue/lvalueResult.h"

// External variables needed for grammar actions
extern int generate_code;
extern int exec_block;
extern int current_loop_exit_label;
extern int last_condition_result;
extern int inline_mode;
extern char* currentScope;

// Funções de ações da gramática para declarações
void handle_var_declaration(const char* type, const char* var_name);
void handle_var_declaration_with_assignment(const char* type, const char* var_name, ExpressionResult* expr);
void handle_for_init_declaration(const char* type, const char* var_name, ExpressionResult* expr);

// Funções de ações da gramática para comandos
void handle_break_statement();
void handle_print_expression(ExpressionResult* expr);
void handle_read_lvalue(LValueResult* lval);

// Funções de ações da gramática para declarações de controle de fluxo
int handle_while_condition(ExpressionResult* condition);
void handle_while_body_end(int start_label, int end_label);
int handle_if_condition(ExpressionResult* condition);
void handle_if_else_part(int else_label);
int get_current_end_label();
void handle_if_end_part(int end_label);

// Funções de ações da gramática para comandos de I/O  
void handle_print_statement();
void handle_read_statement(LValueResult* lval);

// Funções de ações da gramática para atribuições
void handle_assignment(LValueResult* lval, ExpressionResult* expr);
void handle_increment(LValueResult* lval);
void handle_decrement(LValueResult* lval);

// Funções de ações da gramática para controle de fluxo
int handle_if_start(ExpressionResult* condition);
void handle_if_end(int label);
void handle_if_else_start(int label);
void handle_if_else_end(int label);

int handle_while_start(ExpressionResult* condition);
void handle_while_end(int start_label);

// Funções de ações da gramática para criação de expressões
ExpressionResult* create_bool_expression(double value);
ExpressionResult* create_int_expression(double value);
ExpressionResult* create_float_expression(double value);
ExpressionResult* create_string_expression(const char* value);
ExpressionResult* create_lvalue_expression(LValueResult* lval);

// Funções de ações da gramática para operações binárias aritméticas
ExpressionResult* handle_addition(ExpressionResult* left, ExpressionResult* right);
ExpressionResult* handle_subtraction(ExpressionResult* left, ExpressionResult* right);
ExpressionResult* handle_multiplication(ExpressionResult* left, ExpressionResult* right);
ExpressionResult* handle_division(ExpressionResult* left, ExpressionResult* right);
ExpressionResult* handle_exponentiation(ExpressionResult* left, ExpressionResult* right);
ExpressionResult* handle_modulo(ExpressionResult* left, ExpressionResult* right);

// Funções de ações da gramática para operações de comparação
ExpressionResult* handle_less_than(ExpressionResult* left, ExpressionResult* right);
ExpressionResult* handle_greater_than(ExpressionResult* left, ExpressionResult* right);
ExpressionResult* handle_less_equal(ExpressionResult* left, ExpressionResult* right);
ExpressionResult* handle_greater_equal(ExpressionResult* left, ExpressionResult* right);
ExpressionResult* handle_equals(ExpressionResult* left, ExpressionResult* right);

// Funções de ações da gramática para operações lógicas
ExpressionResult* handle_logical_and(ExpressionResult* left, ExpressionResult* right);
ExpressionResult* handle_logical_or(ExpressionResult* left, ExpressionResult* right);
ExpressionResult* handle_unary_minus(ExpressionResult* expr);
ExpressionResult* handle_logical_not(ExpressionResult* expr);

// Funções de ações da gramática para outras expressões
ExpressionResult* handle_function_call(const char* func_name);
ExpressionResult* handle_function_call_with_args(const char* func_name, const char* args);
ExpressionResult* handle_len_expression(ExpressionResult* array_expr);
ExpressionResult* handle_array_literal(const char* element_type);
ExpressionResult* handle_array_literal_with_values(const char* values);

// Funções de ações da gramática para manipulação de tipos
char* create_array_type(const char* base_type);
char* create_2d_array_type(const char* base_type);
char* create_3d_array_type(const char* base_type);

// Funções de ações da gramática para gerenciamento de escopo
void handle_function_start(const char* return_type, const char* func_name);
void handle_function_end();
void handle_block_start();
void handle_block_end();
void handle_for_start();
void handle_for_end();

// Funções de ações da gramática para parâmetros
void handle_parameter_declaration(const char* type, const char* param_name);
void handle_function_parameter(const char* type, const char* param_name);

// Return statement handling
void handle_return_statement(ExpressionResult* expr);

#endif // GRAMMAR_ACTIONS_H
