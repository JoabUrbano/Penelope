#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <stdarg.h>

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
void emit_2d_array_deallocation(const char* var_name, const char* rows_var);

// Funções de conversão de tipos
const char* penelope_to_c_type(const char* penelopeType);
char* convert_penelope_type_to_c(const char* penelopeType);

// Funções de arquivo
int create_directory_if_not_exists(const char* path);
char* get_directory_from_path(const char* filepath);

#endif // CODE_GENERATOR_H
