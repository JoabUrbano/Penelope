#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "../../structs/expression/expressionResult.h"
#include "../../structs/lvalue/lvalueResult.h"

// Contadores globais de erros
extern int semantic_errors;
extern int syntax_errors;

// Controle de execução
extern int exec_block;
extern int last_condition_result;

// Flags para controle de formatação de impressão
extern int is_matrix_element_print;
extern int has_explicit_newline;
extern int last_print_arg_count;

// Funções de relatório de erros
void semantic_error(const char* format, ...);
void print_string(char *str);
void print_newline();

// Funções auxiliares de execução
double power_operation(double base, double exponent);

// Semantic analysis functions for variable management
int validate_variable_declaration(const char* type, const char* var_name);
int validate_variable_assignment(const char* var_name, ExpressionResult* expr);
int validate_array_access(const char* var_name, ExpressionResult* index);
int validate_binary_operation(ExpressionResult* left, ExpressionResult* right, const char* operation);
int validate_unary_operation(ExpressionResult* expr, const char* operation);

// Semantic analysis functions for type checking
int check_type_compatibility(const char* expected_type, const char* actual_type);
char* get_expression_result_type(ExpressionResult* expr);
char* get_binary_operation_result_type(const char* left_type, const char* right_type, const char* operation);

// Semantic analysis functions for control flow
int validate_condition_expression(ExpressionResult* condition);
int validate_loop_increment(LValueResult* lval);

// Symbol table semantic operations
int declare_variable(const char* type, const char* var_name, ExpressionResult* initial_value);
int assign_to_variable(const char* var_name, ExpressionResult* value);
int increment_variable(const char* var_name);
int decrement_variable(const char* var_name);

// Expression evaluation functions
ExpressionResult* evaluate_binary_expression(ExpressionResult* left, ExpressionResult* right, const char* operation);
ExpressionResult* evaluate_unary_expression(ExpressionResult* expr, const char* operation);
ExpressionResult* evaluate_variable_access(const char* var_name);
ExpressionResult* evaluate_array_access(const char* var_name, ExpressionResult* index);

// Helper functions for expression evaluation
int evaluate_boolean_expression(ExpressionResult* expr);
void print_value(ExpressionResult* expr);

#endif // SEMANTICS_H
