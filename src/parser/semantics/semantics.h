#ifndef SEMANTICS_H
#define SEMANTICS_H

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

#endif // SEMANTICS_H
