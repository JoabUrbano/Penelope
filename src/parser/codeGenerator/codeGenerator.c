#include "codeGenerator.h"
#include "../symbolTable/symbolTable.h"
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
int in_main_function = 0;

// Flag global para rastreamento de cadeia else-if é declarada no header

// Função para desabilitar temporariamente a geração de código
void disable_code_generation() {
    generate_code = 0;
}

// Função para habilitar a geração de código  
void enable_code_generation() {
    generate_code = 1;
}

// Função para emitir código C sem quebra de linha automática
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

// Função para emitir código C com quebra de linha automática
void emit_line(const char* format, ...) {
    if (!generate_code) return;
    
    va_list args;
    va_start(args, format);
    
    int remaining = MAX_CODE_SIZE - code_position;
    if (remaining > 0) {
        int written = vsnprintf(generated_code + code_position, remaining, format, args);
        if (written > 0 && written < remaining) {
            code_position += written;
            // Adiciona quebra de linha após o conteúdo
            if (code_position < MAX_CODE_SIZE - 1) {
                generated_code[code_position] = '\n';
                code_position++;
                generated_code[code_position] = '\0';
            }
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

// Função para gerar alocação de array 2D com verificação de necessidade
void emit_2d_array_allocation_if_needed(const char* var_name, const char* rows_var, const char* cols_var) {
    if (!generate_code) return;
    
    emit_line("// Alocação de array 2D %s se necessário", var_name);
    emit_line("if (%s == NULL) {", var_name);
    emit_line("    %s = malloc(%s * sizeof(int*));", var_name, rows_var);
    emit_line("    for (int _i = 0; _i < %s; _i++) {", rows_var);
    emit_line("        %s[_i] = malloc(%s * sizeof(int));", var_name, cols_var);
    emit_line("    }");
    emit_line("}");
}

// Funções de conversão de tipos
char* convert_penelope_type_to_c(const char* penelopeType) {
    if (strcmp(penelopeType, "int") == 0) return "int";
    if (strcmp(penelopeType, "bool") == 0) return "int";  // bool -> int em C
    if (strcmp(penelopeType, "float") == 0) return "float";
    if (strcmp(penelopeType, "string") == 0) return "char*";
    if (strcmp(penelopeType, "int[][]") == 0) return "int**";
    if (strcmp(penelopeType, "float[][]") == 0) return "float**";
    if (strstr(penelopeType, "int[]")) return "int*";
    if (strstr(penelopeType, "float[]")) return "float*";
    if (strstr(penelopeType, "string[]")) return "char**";
    
    // Verifica se é um tipo struct
    StructDefinition* struct_def = find_struct_definition(penelopeType);
    if (struct_def) {
        return strdup(penelopeType); // Tipos struct usam o mesmo nome em C
    }
    
    if (strstr(penelopeType, "int&") != NULL) return strdup("int*"); // referência
    if (strstr(penelopeType, "float&") != NULL) return strdup("float*");
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
    in_main_function = 0;
    
    // Emite apenas cabeçalhos C - main será emitido pela gramática
    emit_line("// Código C gerado a partir de Penelope");
    emit_line("#include <stdio.h>");
    emit_line("#include <stdlib.h>");
    emit_line("#include <string.h>");
    emit_line("#include <math.h>");
}

void finalize_code_generation() {
    // Finaliza qualquer código pendente
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

// Funções de geração de código de expressão
void emit_expression_code(ExpressionResult* expr) {
    if (!generate_code || !expr) return;
    
    if (strcmp(expr->type, "int") == 0) {
        if (expr->c_code) {
            emit_line("printf(\"%%d\", %s);", expr->c_code);
        } else {
            emit_line("printf(\"%%d\", %d);", expr->intVal);
        }
    } else if (strcmp(expr->type, "float") == 0) {
        if (expr->c_code) {
            emit_line("printf(\"%%.6f\", %s);", expr->c_code);
        } else {
            emit_line("printf(\"%%.6f\", %f);", expr->doubleVal);
        }
    } else if (strcmp(expr->type, "bool") == 0) {
        if (expr->c_code) {
            emit_line("printf(\"%%s\", %s ? \"true\" : \"false\");", expr->c_code);
        } else {
            emit_line("printf(\"%%s\", %s);", expr->intVal ? "\"true\"" : "\"false\"");
        }
    } else if (strcmp(expr->type, "string") == 0) {
        if (expr->c_code) {
            emit_line("printf(\"%%s\", %s);", expr->c_code);
        } else {
            emit_line("printf(\"%%s\", %s);", expr->strVal);
        }
    }
}

void emit_assignment_code(LValueResult* lval, ExpressionResult* expr) {
    if (!generate_code || !lval || !expr) return;
    
    if (lval->type == LVALUE_ARRAY_ACCESS) {
        // Auto-aloca array 2D se necessário
        if (lval->dimensionCount == 2) {
            emit_auto_allocate_2d_array(lval->varName);
        }
        
        char array_access[256] = "";
        snprintf(array_access, sizeof(array_access), "%s", lval->varName);
        
        for (int i = 0; i < lval->dimensionCount; i++) {
            char index_str[64];
            snprintf(index_str, sizeof(index_str), "[%s]", lval->indexExpressions[i]);
            strcat(array_access, index_str);
        }
        
        if (inline_mode) {
            emit_inline("%s = %s", array_access, expr->c_code ? expr->c_code : "0");
        } else {
            emit_line("%s = %s;", array_access, expr->c_code ? expr->c_code : "0");
        }
    } else if (lval->type == LVALUE_VAR) {
        // Verifica se é um parâmetro de referência (tipo termina com &)
        Data* var_data = find_variable_in_scopes(lval->varName);
        int is_reference = 0;
        if (var_data && var_data->type) {
            int type_len = strlen(var_data->type);
            if (type_len > 0 && var_data->type[type_len - 1] == '&') {
                is_reference = 1;
            }
        }
        
        if (inline_mode) {
            if (is_reference) {
                emit_inline("*%s = %s", lval->varName, expr->c_code ? expr->c_code : "0");
            } else {
                emit_inline("%s = %s", lval->varName, expr->c_code ? expr->c_code : "0");
            }
        } else {
            if (is_reference) {
                emit_line("*%s = %s;", lval->varName, expr->c_code ? expr->c_code : "0");
            } else {
                emit_line("%s = %s;", lval->varName, expr->c_code ? expr->c_code : "0");
            }
        }
    } else if (lval->type == LVALUE_STRUCT_FIELD) {
        // Atribuição de campo de struct: struct_var.field = value
        if (inline_mode) {
            emit_inline("%s.%s = %s", lval->varName, lval->fieldName, expr->c_code ? expr->c_code : "0");
        } else {
            emit_line("%s.%s = %s;", lval->varName, lval->fieldName, expr->c_code ? expr->c_code : "0");
        }
    }
}

void emit_increment_code(LValueResult* lval) {
    if (!generate_code || !lval) return;
    
    if (inline_mode) {
        emit_inline("%s++", lval->varName);
    } else {
        emit_line("%s++;", lval->varName);
    }
}

void emit_decrement_code(LValueResult* lval) {
    if (!generate_code || !lval) return;
    
    if (inline_mode) {
        emit_inline("%s--", lval->varName);
    } else {
        emit_line("%s--;", lval->varName);
    }
}

// Funções de geração de código de fluxo de controle
void emit_while_start_code(ExpressionResult* condition, int start_label, int end_label) {
    if (!generate_code) return;
    
    emit_line("L%d:", start_label);
    emit_line("// condição do loop while");
    
    if (condition->c_code) {
        emit_line("if (!(%s)) goto L%d;", condition->c_code, end_label);
    } else {
        emit_line("if (!(1)) goto L%d;", end_label);
    }
}

void emit_while_end_code(int start_label, int end_label) {
    if (!generate_code) return;
    
    emit_line("goto L%d;", start_label);
    emit_line("L%d:", end_label);
    emit_line("// fim do loop while");
}

void emit_if_start_code(ExpressionResult* condition, int else_label) {
    if (!generate_code) return;
    
    emit_line("// condição if");
    if (condition->c_code) {
        emit_line("if (%s) {", condition->c_code);
    } else {
        emit_line("if (1) {");
    }
    increase_indent();
}

void emit_if_else_code(int else_label, int end_label) {
    if (!generate_code) return;
    
    decrease_indent();
    emit_code("} else {");
    increase_indent();
}

void emit_if_end_code(int end_label) {
    if (!generate_code) return;
    
    decrease_indent();
    emit_line("}");
    emit_line("// fim if");
}

// Funções de geração de código de E/S
void emit_print_code(ExpressionResult* expr) {
    emit_expression_code(expr);
}

void emit_read_code(LValueResult* lval) {
    if (!generate_code || !lval) return;
    
    if (lval->type == LVALUE_ARRAY_ACCESS) {
        // Auto-aloca array 2D se necessário
        if (lval->dimensionCount == 2) {
            emit_auto_allocate_2d_array(lval->varName);
        }
        
        char array_access[256] = "";
        snprintf(array_access, sizeof(array_access), "%s", lval->varName);
        
        for (int i = 0; i < lval->dimensionCount; i++) {
            char index_str[64];
            snprintf(index_str, sizeof(index_str), "[%s]", lval->indexExpressions[i]);
            strcat(array_access, index_str);
        }
        
        emit_line("scanf(\"%%d\", &%s);", array_access);
    } else if (lval->type == LVALUE_VAR) {
        emit_line("scanf(\"%%d\", &%s);", lval->varName);
    }
}

// Geração de código de declaração de variável
void emit_var_declaration_code(const char* type, const char* var_name) {
    if (!generate_code) return;
    
    char* c_type = convert_penelope_type_to_c(type);
    if (strstr(type, "[][]") != NULL) {
        // Para arrays 2D, apenas gera a declaração do ponteiro
        // A alocação será feita quando as dimensões forem conhecidas
        emit_line("%s %s = NULL; // Declaração de Array 2D", c_type, var_name);
    } else if (strstr(type, "[]") != NULL) {
        emit_line("%s %s = NULL; // Declaração de Array", c_type, var_name);
    } else {
        emit_line("%s %s;", c_type, var_name);
    }
}

void emit_var_assignment_code(const char* type, const char* var_name, ExpressionResult* expr) {
    if (!generate_code) return;
    
    // Verifica se estamos em uma função que retorna um array e isso é um literal de array
    extern char* current_function_return_type;
    int in_array_returning_function = (current_function_return_type && strstr(current_function_return_type, "[]") != NULL);
    
    char* c_type = convert_penelope_type_to_c(type);
    if (expr->c_code) {
        // Verifica se esta é uma inicialização de literal de array
        if (strstr(type, "[]") != NULL && expr->c_code[0] == '{') {
            // Para literais de array em funções que retornam arrays, usa alocação dinâmica
            if (in_array_returning_function) {
                if (strstr(type, "int[]") != NULL) {
                    // Analisa literal de array para contar elementos e obter valores
                    char* literal = expr->c_code;
                    
                    // Conta elementos no literal de array
                    int element_count = 1; // Pelo menos 1 elemento
                    for (char* p = literal; *p; p++) {
                        if (*p == ',') element_count++;
                    }
                    
                    // Extrai valores do literal
                    emit_line("int* %s = malloc(%d * sizeof(int));", var_name, element_count);
                    
                    // Inicializa elementos
                    char* literal_copy = strdup(literal);
                    char* p = literal_copy;
                    
                    // Remove chaves
                    if (*p == '{') p++;
                    char* end = p + strlen(p) - 1;
                    if (*end == '}') *end = '\0';
                    
                    // Processa cada elemento
                    char* token = strtok(p, ",");
                    int index = 0;
                    while (token != NULL && index < element_count) {
                        // Remove espaços em branco
                        while (*token == ' ') token++;
                        char* token_end = token + strlen(token) - 1;
                        while (token_end > token && *token_end == ' ') token_end--;
                        *(token_end + 1) = '\0';
                        
                        emit_line("%s[%d] = %s;", var_name, index, token);
                        token = strtok(NULL, ",");
                        index++;
                    }
                    
                    free(literal_copy);
                } else if (strstr(type, "float[]") != NULL) {
                    // Analisa literal de array para contar elementos e obter valores
                    char* literal = expr->c_code;
                    
                    // Conta elementos no literal de array
                    int element_count = 1; // Pelo menos 1 elemento
                    for (char* p = literal; *p; p++) {
                        if (*p == ',') element_count++;
                    }
                    
                    // Extrai valores do literal
                    emit_line("float* %s = malloc(%d * sizeof(float));", var_name, element_count);
                    
                    // Inicializa elementos
                    char* literal_copy = strdup(literal);
                    char* p = literal_copy;
                    
                    // Remove chaves
                    if (*p == '{') p++;
                    char* end = p + strlen(p) - 1;
                    if (*end == '}') *end = '\0';
                    
                    // Processa cada elemento
                    char* token = strtok(p, ",");
                    int index = 0;
                    while (token != NULL && index < element_count) {
                        // Remove espaços em branco
                        while (*token == ' ') token++;
                        char* token_end = token + strlen(token) - 1;
                        while (token_end > token && *token_end == ' ') token_end--;
                        *(token_end + 1) = '\0';
                        
                        emit_line("%s[%d] = %s;", var_name, index, token);
                        token = strtok(NULL, ",");
                        index++;
                    }
                    
                    free(literal_copy);
                } else {
                    // Fallback para sintaxe de ponteiro com malloc (arrays genéricos)
                    char* literal = expr->c_code;
                    
                    // Conta elementos no literal de array
                    int element_count = 1; // Pelo menos 1 elemento
                    for (char* p = literal; *p; p++) {
                        if (*p == ',') element_count++;
                    }
                    
                    emit_line("%s %s = malloc(%d * sizeof(int)); // Array genérico", c_type, var_name, element_count);
                }
            } else {
                // Para funções que não retornam, usa alocação em pilha como antes
                if (strstr(type, "int[]") != NULL) {
                    emit_line("int %s[] = %s;", var_name, expr->c_code);
                } else if (strstr(type, "float[]") != NULL) {
                    emit_line("float %s[] = %s;", var_name, expr->c_code);
                } else {
                    // Fallback para sintaxe de ponteiro
                    emit_line("%s %s = %s;", c_type, var_name, expr->c_code);
                }
            }
        } else if (strstr(type, "[]") != NULL && strstr(expr->c_code, "()") != NULL) {
            // Esta é uma chamada de função que retorna um array
            // Para chamadas de função que retornam arrays, precisamos declarar a variável e atribuir
            if (strstr(type, "int[]") != NULL) {
                emit_line("int* %s = %s;", var_name, expr->c_code);
            } else if (strstr(type, "float[]") != NULL) {
                emit_line("float* %s = %s;", var_name, expr->c_code);
            } else {
                // Fallback para sintaxe de ponteiro
                emit_line("%s %s = %s;", c_type, var_name, expr->c_code);
            }
        } else {
            emit_line("%s %s = %s;", c_type, var_name, expr->c_code);
        }
    } else {
        if (strcmp(expr->type, "int") == 0) {
            emit_line("%s %s = %d;", c_type, var_name, expr->intVal);
        } else if (strcmp(expr->type, "float") == 0) {
            emit_line("%s %s = %f;", c_type, var_name, expr->doubleVal);
        } else if (strcmp(expr->type, "bool") == 0) {
            emit_line("%s %s = %d;", c_type, var_name, expr->intVal);
        } else if (strcmp(expr->type, "string") == 0) {
            emit_line("%s %s = %s;", c_type, var_name, expr->strVal ? expr->strVal : "\"\"");
        }
    }
}

void emit_for_init_code(const char* type, const char* var_name, ExpressionResult* expr) {
    if (!generate_code) return;
    
    char* c_type = convert_penelope_type_to_c(type);
    if (expr->c_code) {
        emit_inline("%s %s = %s", c_type, var_name, expr->c_code);
    } else {
        if (strcmp(expr->type, "int") == 0) {
            emit_inline("%s %s = %d", c_type, var_name, expr->intVal);
        } else if (strcmp(expr->type, "float") == 0) {
            emit_inline("%s %s = %f", c_type, var_name, expr->doubleVal);
        } else if (strcmp(expr->type, "bool") == 0) {
            emit_inline("%s %s = %d", c_type, var_name, expr->intVal);
        } else if (strcmp(expr->type, "string") == 0) {
            emit_inline("%s %s = %s", c_type, var_name, expr->strVal ? expr->strVal : "\"\"");
        }
    }
}

// Função para inferir nomes de variáveis de dimensão baseado no nome do array
void get_dimension_variable_names(const char* array_name, char* rows_var, char* cols_var) {
    // Padrão 2: matrizSoma -> linhas1, colunas1 (mesmas dimensões das matrizes de entrada)
    if (strcmp(array_name, "matrizSoma") == 0) {
        strcpy(rows_var, "linhas1");
        strcpy(cols_var, "colunas1");
        return;
    }
    
    // Padrão 3: matrizProduto -> linhasResultado, colunasResultado
    if (strcmp(array_name, "matrizProduto") == 0) {
        strcpy(rows_var, "linhasResultado");
        strcpy(cols_var, "colunasResultado");
        return;
    }
    
    // Padrão 1: matriz{N} -> linhas{N}, colunas{N}
    if (strncmp(array_name, "matriz", 6) == 0) {
        const char* num_part = array_name + 6; // parte após "matriz"
        snprintf(rows_var, 64, "linhas%s", num_part);
        snprintf(cols_var, 64, "colunas%s", num_part);
        return;
    }
    
    // Padrão 4: {name} -> {name}_rows, {name}_cols
    snprintf(rows_var, 64, "%s_rows", array_name);
    snprintf(cols_var, 64, "%s_cols", array_name);
}

// Função para verificar se um array 2D precisa ser alocado antes do acesso
void emit_auto_allocate_2d_array(const char* var_name) {
    if (!generate_code) return;
    
    char rows_var[64], cols_var[64];
    get_dimension_variable_names(var_name, rows_var, cols_var);
    
    emit_line("// Auto-alocação de array 2D %s se necessário", var_name);
    emit_line("if (%s == NULL) {", var_name);
    emit_line("    %s = malloc(%s * sizeof(int*));", var_name, rows_var);
    emit_line("    for (int _i = 0; _i < %s; _i++) {", rows_var);
    emit_line("        %s[_i] = malloc(%s * sizeof(int));", var_name, cols_var);
    emit_line("    }");
    emit_line("}");
}

// ========== GERAÇÃO DE CÓDIGO DE STRUCT ==========

void emit_struct_definition(const char* struct_name, StructField* fields) {
    if (!generate_code) return;
    
    emit_line("typedef struct {");
    increase_indent();
    
    // Emite campos em ordem reversa (já que são armazenados como lista ligada)
    StructField* field_array[100];
    int field_count = 0;
    
    StructField* current = fields;
    while (current && field_count < 100) {
        field_array[field_count++] = current;
        current = current->next;
    }
    
    // Emite campos na ordem original
    for (int i = field_count - 1; i >= 0; i--) {
        char* c_type = convert_penelope_type_to_c(field_array[i]->type);
        emit_line("%s %s;", c_type, field_array[i]->name);
    }
    
    decrease_indent();
    emit_line("} %s;", struct_name);
    emit_line("");
}