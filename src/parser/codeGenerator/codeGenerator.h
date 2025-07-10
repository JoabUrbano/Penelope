#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <stdarg.h>
#include "../../structs/expression/expressionResult.h"
#include "../../structs/lvalue/lvalueResult.h"
#include "../symbolTable/symbolTable.h"

// Configurações de geração de código
#define MAX_CODE_SIZE 10000

extern char generated_code[MAX_CODE_SIZE];
extern int code_position;
extern int indent_level;
extern int generate_code;
extern int label_counter;
extern int inline_mode;

// Controle de loops para break/continue
extern int current_loop_exit_label;

// Controle de função atual
extern int in_main_function;

// Controle de contexto else if
extern int in_else_if_context;

// Rastreamento de contexto de função para alocação apropriada de array
extern char* current_function_return_type;

// Controle de geração de código
void disable_code_generation();
void enable_code_generation();

// Funções de geração de código
void emit_code(const char* format, ...);
void emit_line(const char* format, ...);
void emit_inline(const char* format, ...);
int generate_label();

// Funções auxiliares de indentação e inicialização
void increase_indent();
void decrease_indent();
void init_code_generation();
void finalize_code_generation();
void set_inline_mode(int mode);

// Funções de geração específicas
void emit_2d_array_allocation(const char* var_name, const char* rows_var, const char* cols_var);
void emit_2d_array_allocation_if_needed(const char* var_name, const char* rows_var, const char* cols_var);
void emit_auto_allocate_2d_array(const char* var_name);
void emit_2d_array_deallocation(const char* var_name, const char* rows_var);

// Funções de conversão de tipos
const char* penelope_to_c_type(const char* penelopeType);
char* convert_penelope_type_to_c(const char* penelopeType);

// Funções de arquivo
int create_directory_if_not_exists(const char* path);
char* get_directory_from_path(const char* filepath);

// Funções de geração de código para expressões
void emit_expression_code(ExpressionResult* expr);
void emit_assignment_code(LValueResult* lval, ExpressionResult* expr);
void emit_increment_code(LValueResult* lval);
void emit_decrement_code(LValueResult* lval);

// Funções de geração de código para controle de fluxo
void emit_while_start_code(ExpressionResult* condition, int start_label, int end_label);
void emit_while_end_code(int start_label, int end_label);
void emit_if_start_code(ExpressionResult* condition, int else_label);
void emit_if_else_code(int else_label, int end_label);
void emit_if_end_code(int end_label);

// Funções de geração de código para entrada e saída
void emit_print_code(ExpressionResult* expr);
void emit_read_code(LValueResult* lval);

// Funções de geração de código para declaração de variáveis
void emit_var_declaration_code(const char* type, const char* var_name);
void emit_var_assignment_code(const char* type, const char* var_name, ExpressionResult* expr);
void emit_for_init_code(const char* type, const char* var_name, ExpressionResult* expr);

// Funções de geração de código para controle de fluxo
void emit_while_start_code(ExpressionResult* condition, int start_label, int end_label);
void emit_while_end_code(int start_label, int end_label);
void emit_if_start_code(ExpressionResult* condition, int else_label);
void emit_if_else_code(int else_label, int end_label);
void emit_if_end_code(int end_label);
void emit_for_start_code(const char* var_name, ExpressionResult* start_expr, ExpressionResult* end_expr, int increment_label, int condition_label, int end_label);
void emit_for_end_code(LValueResult* increment_lval, int increment_label, int condition_label, int end_label);

// Funções de geração de código de struct
void emit_struct_definition(const char* struct_name, StructField* fields);
void emit_struct_field_access(const char* struct_var, const char* field_name, const char* field_type);

#endif // CODE_GENERATOR_H
