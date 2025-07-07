#include "codeGenerator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

// Variáveis globais para geração de código
char generated_code[MAX_CODE_SIZE];
int code_position = 0;
int indent_level = 0;
int generate_code = 1;
int label_counter = 0;
int inline_mode = 0;
int current_loop_exit_label = -1;

void emit_code(const char* format, ...) {
    if (!generate_code) return;
    
    va_list args;
    va_start(args, format);
    
    int remaining = MAX_CODE_SIZE - code_position;
    if (remaining > 0) {
        int written = vsnprintf(generated_code + code_position, remaining, format, args);
        if (written > 0 && written < remaining) {
            code_position += written;
        }
    }
    
    va_end(args);
}

void emit_line(const char* format, ...) {
    if (!generate_code) return;
    
    va_list args;
    va_start(args, format);
    
    int remaining = MAX_CODE_SIZE - code_position;
    if (remaining > 0) {
        int written = vsnprintf(generated_code + code_position, remaining, format, args);
        if (written > 0 && written < remaining) {
            code_position += written;
        }
    }
    
    va_end(args);
}

void emit_inline(const char* format, ...) {
    if (!generate_code || !inline_mode) return;
    
    va_list args;
    va_start(args, format);
    
    int remaining = MAX_CODE_SIZE - code_position;
    if (remaining > 0) {
        int written = vsnprintf(generated_code + code_position, remaining, format, args);
        if (written > 0 && written < remaining) {
            code_position += written;
        }
    }
    
    va_end(args);
}

int generate_label() {
    return ++label_counter;
}

// Função para gerar código de alocação de array 2D
void emit_2d_array_allocation(const char* var_name, const char* rows_var, const char* cols_var) {
    emit_line("// Alocação de memória para array 2D %s", var_name);
    emit_line("%s = malloc(%s * sizeof(int*));", var_name, rows_var);
    emit_line("for (int _i = 0; _i < %s; _i++) {", rows_var);
    emit_line("    %s[_i] = malloc(%s * sizeof(int));", var_name, cols_var);
    emit_line("}");
}

// Função para gerar código de liberação de array 2D
void emit_2d_array_deallocation(const char* var_name, const char* rows_var) {
    emit_line("// Liberação de memória para array 2D %s", var_name);
    emit_line("for (int _i = 0; _i < %s; _i++) {", rows_var);
    emit_line("    free(%s[_i]);", var_name);
    emit_line("}");
    emit_line("free(%s);", var_name);
}

// Funções de conversão de tipos
char* convert_penelope_type_to_c(const char* penelopeType) {
    if (strcmp(penelopeType, "int") == 0) return "int";
    if (strcmp(penelopeType, "bool") == 0) return "int";  // bool -> int in C
    if (strcmp(penelopeType, "float") == 0) return "float";
    if (strcmp(penelopeType, "string") == 0) return "char*";
    if (strcmp(penelopeType, "int[][]") == 0) return "int**";
    if (strcmp(penelopeType, "float[][]") == 0) return "float**";
    if (strstr(penelopeType, "int[]")) return "int*";
    if (strstr(penelopeType, "float[]")) return "float*";
    if (strstr(penelopeType, "string[]")) return "char**";
    return "void"; // fallback padrão
}

const char* penelope_to_c_type(const char* penelopeType) {
    return convert_penelope_type_to_c(penelopeType);
}

// Funções auxiliares de indentação
void increase_indent() {
    indent_level++;
}

void decrease_indent() {
    if (indent_level > 0) {
        indent_level--;
    }
}

// Funções de controle de modo inline
void set_inline_mode(int mode) {
    inline_mode = mode;
}

// Inicialização e finalização
void init_code_generation() {
    code_position = 0;
    indent_level = 0;
    label_counter = 0;
    memset(generated_code, 0, MAX_CODE_SIZE);
    
    // Emite cabeçalhos C e início da função main
    emit_line("#include <stdio.h>");
    emit_line("#include <stdlib.h>");
    emit_line("#include <string.h>");
    emit_line("");
}

void finalize_code_generation() {
    // Adiciona limpeza de arrays 2D no final da função main
    if (generate_code) {
        emit_line("// Limpeza de memória dos arrays 2D");
        emit_line("// Note: Em um código real, você deveria liberar a memória aqui");
        emit_line("// Exemplo para matriz1: emit_2d_array_deallocation(\"matriz1\", \"linhas1\");");
    }
    
    // Fecha qualquer função main aberta se necessário
    if (code_position > 0) {
        generated_code[code_position] = '\0';
    }
}

// Função para criar diretório se não existir
int create_directory_if_not_exists(const char* path) {
    struct stat st = {0};
    
    // Verifica se o diretório já existe
    if (stat(path, &st) == -1) {
        // Diretório não existe, tenta criar
        if (mkdir(path, 0755) == -1) {
            if (errno != EEXIST) {  // Ignora erro se diretório foi criado por outro processo
                return 0;  // Falha ao criar
            }
        }
    }
    return 1;  // Sucesso (diretório existe ou foi criado)
}

// Função para extrair o diretório do caminho do arquivo
char* get_directory_from_path(const char* filepath) {
    if (!filepath) return NULL;
    
    char* path_copy = strdup(filepath);
    char* last_slash = strrchr(path_copy, '/');
    
    if (last_slash) {
        *last_slash = '\0';  // Termina a string no último '/'
        return path_copy;
    } else {
        free(path_copy);
        return strdup(".");  // Diretório atual se não há '/'
    }
}
