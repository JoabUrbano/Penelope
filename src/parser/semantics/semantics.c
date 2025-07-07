#include "semantics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

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
    // Não gera saída em tempo de execução - apenas gera código C
    // printf("\n");
}

double power_operation(double base, double exponent) {
    return pow(base, exponent);
}
