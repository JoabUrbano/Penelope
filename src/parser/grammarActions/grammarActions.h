#ifndef GRAMMAR_ACTIONS_H
#define GRAMMAR_ACTIONS_H

#include "../parserCommon/parser_common.h"

// Grammar action functions for declarations
void handle_var_declaration(const char* type, const char* var_name);
void handle_var_declaration_with_assignment(const char* type, const char* var_name, ExpressionResult* expr);
void handle_for_init_declaration(const char* type, const char* var_name, ExpressionResult* expr);

// Grammar action functions for statements
void handle_break_statement();
void handle_print_expression(ExpressionResult* expr);
void handle_read_lvalue(LValueResult* lval);

// Grammar action functions for assignments
void handle_assignment(LValueResult* lval, ExpressionResult* expr);
void handle_increment(LValueResult* lval);
void handle_decrement(LValueResult* lval);

// Grammar action functions for control flow
int handle_if_start(ExpressionResult* condition);
void handle_if_end(int label);
void handle_if_else_start(int label);
void handle_if_else_end(int label);

int handle_while_start(ExpressionResult* condition);
void handle_while_end(int start_label);

// Grammar action functions for expressions
ExpressionResult* create_bool_expression(double value);
ExpressionResult* create_int_expression(double value);
ExpressionResult* create_float_expression(double value);
ExpressionResult* create_string_expression(const char* value);
ExpressionResult* create_lvalue_expression(LValueResult* lval);

// Grammar action functions for binary operations
ExpressionResult* handle_addition(ExpressionResult* left, ExpressionResult* right);
ExpressionResult* handle_subtraction(ExpressionResult* left, ExpressionResult* right);
ExpressionResult* handle_multiplication(ExpressionResult* left, ExpressionResult* right);
ExpressionResult* handle_division(ExpressionResult* left, ExpressionResult* right);
ExpressionResult* handle_exponentiation(ExpressionResult* left, ExpressionResult* right);

// Grammar action functions for comparison operations
ExpressionResult* handle_less_than(ExpressionResult* left, ExpressionResult* right);
ExpressionResult* handle_greater_than(ExpressionResult* left, ExpressionResult* right);
ExpressionResult* handle_less_equal(ExpressionResult* left, ExpressionResult* right);
ExpressionResult* handle_greater_equal(ExpressionResult* left, ExpressionResult* right);
ExpressionResult* handle_equals(ExpressionResult* left, ExpressionResult* right);

// Grammar action functions for logical operations
ExpressionResult* handle_logical_and(ExpressionResult* left, ExpressionResult* right);
ExpressionResult* handle_logical_or(ExpressionResult* left, ExpressionResult* right);
ExpressionResult* handle_unary_minus(ExpressionResult* expr);

// Grammar action functions for function calls and len
ExpressionResult* handle_function_call(const char* func_name);
ExpressionResult* handle_len_expression(ExpressionResult* expr);
ExpressionResult* handle_array_literal(const char* element_type);

// Grammar action functions for type handling
char* create_array_type(const char* base_type);
char* create_2d_array_type(const char* base_type);
char* create_3d_array_type(const char* base_type);

// Grammar action functions for scope management
void handle_function_start(const char* return_type, const char* func_name);
void handle_function_end();
void handle_block_start();
void handle_block_end();
void handle_for_start();
void handle_for_end();

// Grammar action functions for parameters
void handle_parameter_declaration(const char* type, const char* param_name);

#endif // GRAMMAR_ACTIONS_H
