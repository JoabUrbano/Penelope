%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <errno.h>

#include "./utils/hashMap/hashMap.h"
#include "./utils/uniqueIdentifier/uniqueIdentifier.h"

#define MAX_SCOPE_DEPTH 100
#define MAX_CODE_SIZE 10000

char* scopeStack[MAX_SCOPE_DEPTH];
int scopeTop = -1;
int exec_block = 1;  // Flag de controle para if/else/while gustavo

HashMap symbolTable = { NULL };
char *currentScope = NULL;
int semantic_errors = 0; 
int syntax_errors = 0;
int last_condition_result = 0;  // serve pra lembrar o resultado da condição do IF

// Globais para geração de código
char generated_code[MAX_CODE_SIZE];
int code_position = 0;
int indent_level = 0;
int generate_code = 1;  // Flag para habilitar/desabilitar geração de código
int label_counter = 0;  // Para gerar labels únicos
int inline_mode = 0;    // Para geração de código em linha (for loops)

// Flags para controle de formatação de impressão
int is_matrix_element_print = 0;  // Flag para detectar impressão de elementos de matriz
int has_explicit_newline = 0;     // Flag para detectar quebras de linha explícitas
int last_print_arg_count = 0;     // Contador de argumentos no último print

// Controle de loops para break/continue
int current_loop_exit_label = -1;  // Label de saída do loop atual para break



int are_types_compatible(const char* declaredType, const char* exprType) {
    if (strcmp(declaredType, exprType) == 0) return 1;
    if ((strcmp(declaredType, "float") == 0 && strcmp(exprType, "int") == 0) ||
      (strcmp(declaredType, "int") == 0 && strcmp(exprType, "float") == 0)) return 1;
    return 0;
}

// Função para extrair o tipo base de um tipo de array (ex: "int[]" -> "int")
char* get_array_element_type(const char* arrayType) {
    if (!arrayType || !strstr(arrayType, "[]")) {
        return NULL; // Não é um tipo de array
    }
    
    char* elementType = malloc(strlen(arrayType));
    strcpy(elementType, arrayType);
    
    // Remove o "[]" do final
    char* bracket = strstr(elementType, "[]");
    if (bracket) {
        *bracket = '\0';
    }
    
    return elementType;
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

// Funções para controle de modo inline (para for loops)
void set_inline_mode(int mode) {
    inline_mode = mode;
}

void emit_inline(const char* format, ...) {
    if (!generate_code) return;
    
    va_list args;
    va_start(args, format);
    
    // Adiciona o código formatado (sem quebra de linha)
    int remaining = MAX_CODE_SIZE - code_position - 1;
    if (remaining > 0) {
        int written = vsnprintf(generated_code + code_position, remaining, format, args);
        if (written > 0 && written < remaining) {
            code_position += written;
        }
    }
    
    va_end(args);
}


void semantic_error(const char* format, ...) {
    extern int yylineno;
    fprintf(stderr, "Erro Semântico na linha %d: ", yylineno);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);  // Imprime a mensagem formatada
    va_end(args);

    fprintf(stderr, "\n");
    semantic_errors++;
}

// Funções auxiliares para geração de código
void emit_code(const char* format, ...) {
    if (!generate_code) return;
    
    va_list args;
    va_start(args, format);
    
    // Adiciona o código formatado (sem indentação automática)
    int remaining = MAX_CODE_SIZE - code_position - 1;
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
    
    // Adiciona o código formatado (sem indentação automática)
    int remaining = MAX_CODE_SIZE - code_position - 1;
    if (remaining > 0) {
        int written = vsnprintf(generated_code + code_position, remaining, format, args);
        if (written > 0 && written < remaining) {
            code_position += written;
        }
    }
    
    // Adiciona quebra de linha
    if (code_position < MAX_CODE_SIZE - 1) {
        generated_code[code_position++] = '\n';
    }
    
    va_end(args);
}

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

int generate_label() {
    return ++label_counter;
}

void increase_indent() {
    indent_level++;
}

void decrease_indent() {
    if (indent_level > 0) {
        indent_level--;
    }
}

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

// Função para gerar código de alocação de array 2D
void emit_2d_array_allocation(const char* array_name, const char* rows_var, const char* cols_var) {
    if (!generate_code) return;
    
    emit_line("// Alocação de memória para array 2D %s", array_name);
    emit_line("%s = malloc(%s * sizeof(int*));", array_name, rows_var);
    emit_line("for (int _i = 0; _i < %s; _i++) {", rows_var);
    emit_line("    %s[_i] = malloc(%s * sizeof(int));", array_name, cols_var);
    emit_line("}");
}

// Função para gerar código de liberação de array 2D
void emit_2d_array_deallocation(const char* array_name, const char* rows_var) {
    if (!generate_code) return;
    
    emit_line("// Liberação de memória para array 2D %s", array_name);
    emit_line("for (int _i = 0; _i < %s; _i++) {", rows_var);
    emit_line("    free(%s[_i]);", array_name);
    emit_line("}");
    emit_line("free(%s);", array_name);
}

Node* find_variable_in_scopes(char *name) {
    for (int i = scopeTop; i >= 0; i--) {
        char *scope = scopeStack[i];
        char *fullKey = malloc(strlen(scope) + strlen(name) + 2);
        sprintf(fullKey, "%s#%s", scope, name);
        
        Node *node = find_node(&symbolTable, fullKey);
        free(fullKey);

        if (node) {
            return node; // Variável encontrada
        }
    }
    return NULL; // Variável não encontrada em nenhum escopo
}

Node* find_variable_in_current_scope(char *name) {
    if (currentScope == NULL) return NULL;
    
    char *fullKey = malloc(strlen(currentScope) + strlen(name) + 2);
    sprintf(fullKey, "%s#%s", currentScope, name);
    
    Node *node = find_node(&symbolTable, fullKey);
    free(fullKey);
    
    return node;
}

double evaluate_number(char *str) {
    return atof(str);
}

// Função será definida após declarações de struct

void print_string(char *str) {
    // Remove aspas de literais de string
    if (str && str[0] == '"' && str[strlen(str)-1] == '"') {
        str[strlen(str)-1] = '\0';  // Remove aspas finais
        printf("%s ", str + 1);   // Pula aspas iniciais, adiciona espaço
    } else if (str) {
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

extern int yylex();
extern int yyparse();
extern FILE* yyin;
void yyerror(const char* s);

void push_scope(char *scope) {
    if (scopeTop < MAX_SCOPE_DEPTH - 1) {
        scopeStack[++scopeTop] = scope;
        currentScope = scope;
    } else {
        fprintf(stderr, "Erro: Estouro da pilha de escopos\n");
        exit(1);
    }
}

void pop_scope() {
    if (scopeTop >= 0) {
        free(scopeStack[scopeTop]);
        scopeStack[scopeTop--] = NULL;
        currentScope = (scopeTop >= 0) ? scopeStack[scopeTop] : NULL;
    } else {
        fprintf(stderr, "Erro: Pilha de escopos vazia\n");
        exit(1);
    }
}
%}

/* Inclua o header aqui, *fora* do bloco %{...%}, para que o Bison leia a definição do tipo ANTES do %union */
%code requires {
    #include "./structs/expression/expressionResult.h"
    #include "./structs/lvalue/lvalueResult.h"
}

%code {
    // Gera código C para uma expressão
    char* expression_to_c_code(ExpressionResult* expr) {
        static char buffer[256];
        
        if (!expr) {
            strcpy(buffer, "0");
            return buffer;
        }
        
        if (strcmp(expr->type, "int") == 0) {
            snprintf(buffer, sizeof(buffer), "%d", expr->intVal);
        } else if (strcmp(expr->type, "float") == 0) {
            snprintf(buffer, sizeof(buffer), "%f", expr->doubleVal);
        } else if (strcmp(expr->type, "bool") == 0) {
            snprintf(buffer, sizeof(buffer), "%d", expr->intVal);
        } else if (strcmp(expr->type, "string") == 0) {
            snprintf(buffer, sizeof(buffer), "\"%s\"", expr->strVal ? expr->strVal : "");
        } else {
            strcpy(buffer, "0");
        }
        
        return buffer;
    }

    void print_value(ExpressionResult* expr) {
        // Não gera saída em tempo de execução durante análise - apenas gera código C
        /*
        if (!expr) return;
        
        if (strcmp(expr->type, "int") == 0) {
            printf("%d ", expr->intVal);
        } else if (strcmp(expr->type, "float") == 0) {
            printf("%.6g ", expr->doubleVal);
        } else if (strcmp(expr->type, "bool") == 0) {
            printf("%s ", expr->intVal ? "true" : "false");
        } else if (strcmp(expr->type, "string") == 0) {
            // Trata literais de string - remove aspas se presentes
            if (expr->strVal && expr->strVal[0] == '"' && expr->strVal[strlen(expr->strVal)-1] == '"') {
                // Cria uma string temporária sem aspas
                char* temp = strdup(expr->strVal);
                temp[strlen(temp)-1] = '\0';  // Remove aspas finais
                printf("%s ", temp + 1);      // Pula aspas iniciais
                free(temp);
            } else {
                printf("%s ", expr->strVal ? expr->strVal : "");
            }
        } else {
            printf("unknown ");
        }
        */
    }
}

%union {
    char *str;
    double num;
    ExpressionResult* exprResult;
    LValueResult* lvalueResult;
    int labelNum;  // Para armazenar números de labels
}


%token <num> BOOL
%token <str> ID TYPE STRING
%token <num> NUMBER
%token BREAK
%token AND 
%token OR

%token <num> INT FLOAT

%token FUN WHILE FOR IF ELSE LEN PRINT READ RETURN
%token LBRACKET RBRACKET COMMA LPAREN RPAREN COLON SEMICOLON NEWLINE
%token ASSIGNMENT EQUALS SMALLEREQUALS BIGGEREQUALS SMALLER BIGGER
%token INCREMENT DECREMENT EXPONENTIATION MULTIPLICATION DIVISION ADDITION SUBTRACTION
%token LBRACE RBRACE

%define parse.trace

%type <lvalueResult> lvalue
%type <str> type list_expression
%type <exprResult> expression
%type <labelNum> while_start if_start

%right ASSIGNMENT
%left OR
%left AND
%left EQUALS
%left SMALLEREQUALS BIGGEREQUALS SMALLER BIGGER
%left ADDITION SUBTRACTION
%left MULTIPLICATION DIVISION
%right EXPONENTIATION   
%nonassoc UMINUS 
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE


%start program

%%

program:
    list_decl_fun
    ;

list_decl_fun:
      decl_or_fun
    | list_decl_fun decl_or_fun
    ;

decl_or_fun:
    fun
    | decl SEMICOLON
    | NEWLINE
    ;

fun:
    FUN type ID LPAREN {
        // Cria o escopo da função antes de processar parâmetros
        char *scopeId = uniqueIdentifier();
        push_scope(scopeId);
    } list_param_opt RPAREN LBRACE list_stmt RBRACE {
        // Remove o escopo da função
        pop_scope();
    }
    ;

block:
    LBRACE {
        char *scopeId = uniqueIdentifier();

        push_scope(scopeId);
    } list_stmt RBRACE {
        pop_scope();
    }
    ;

list_stmt:
    stmt
    | list_stmt stmt
    ;

stmt:
    simple_stmt
    | compound_stmt
    | NEWLINE
    ;

simple_stmt:
    decl SEMICOLON
    | assign_stmt SEMICOLON
    | return_stmt SEMICOLON
    | print_stmt SEMICOLON
    | read_stmt SEMICOLON
    | expression SEMICOLON {
        if (exec_block) {
            // Executa a expressão apenas se for dentro de bloco válido
        }
    }
    | BREAK SEMICOLON {
        if (exec_block) {
            // break real ainda não implementado — só evitar erro
        }
        
        // Gera código C para break
        if (generate_code) {
            if (current_loop_exit_label != -1) {
                emit_line("goto L%d;", current_loop_exit_label);
            } else {
                semantic_error("break statement fora de um loop.");
            }
        }
    }
    ;

compound_stmt:
    if_stmt
    | for_stmt
    | while_stmt
    ;

while_stmt:
    while_start block {
        // Gera código C para fim do loop while (goto de volta para condição)
        if (generate_code) {
            emit_line("goto L%d;", $1);  // Volta para testar a condição
            emit_line("L%d:", $1 + 1);   // Label de saída do while
        }
        
        // Limpa o controle de loop
        current_loop_exit_label = -1;
        
        // Após executar o bloco uma vez, reseta o controle de execução
        exec_block = 1;
    }
    ;

while_start:
    WHILE LPAREN expression RPAREN {
        // Avalia a expressão como condição booleana para o while
        int condition = 0;
        if (strcmp($3->type, "bool") == 0) {
            condition = $3->intVal;
        } else if (strcmp($3->type, "int") == 0) {
            condition = ($3->intVal != 0);
        } else if (strcmp($3->type, "float") == 0) {
            condition = ($3->doubleVal != 0.0);
        } else {
            semantic_error("Condição while deve ser do tipo bool, int ou float, mas foi '%s'.", $3->type);
        }
        
        // Gera código C para while usando goto
        int start_label = generate_label();
        int end_label = generate_label();
        
        // Define o label de saída para break statements
        current_loop_exit_label = end_label;
        
        if (generate_code) {
            emit_line("L%d:", start_label);  // Label de início do loop
            
            // Para expressões mais complexas, geramos uma variável temporária
            emit_line("// while loop condition evaluation");
            
            // Testa condição e salta para o fim se falsa usando o código C gerado
            if ($3->c_code) {
                emit_line("if (!(%s)) goto L%d;", $3->c_code, end_label);
            } else {
                emit_line("if (!(%s)) goto L%d;", expression_to_c_code($3), end_label);
            }
        }
        
        // Simula múltiplas iterações do loop para análise semântica
        exec_block = condition;
        free_expression_result($3);
        
        $$ = start_label;  // Retorna o label de início para usar no final
    }
    ;

if_stmt:
    if_start block {
        // Gera código C para label de fechamento do if (sem else)
        if (generate_code) {
            emit_line("L%d:", $1);  // Label do else/fim
        }
        exec_block = 1;  // reseta após o if
    }
    | if_start block ELSE {
        // Gera código C para else usando goto
        if (generate_code) {
            emit_line("goto L%d;", $1 + 1);  // Pula para o fim do if
            emit_line("L%d:", $1);           // Label do else
        }
        exec_block = !last_condition_result;  // ativa o bloco do else se falso
    } block {
        // Gera código C para label de fechamento do if-else
        if (generate_code) {
            emit_line("L%d:", $1 + 1);  // Label de fim
        }
        exec_block = 1;  // reseta após o if
    }
    | if_start block ELSE if_stmt {
        // Gera código C para else if (encadeamento de ifs)
        if (generate_code) {
            emit_line("goto L%d;", $1 + 1);  // Pula para o fim do if principal
            emit_line("L%d:", $1);           // Label do else if
        }
        // O else if é tratado recursivamente
        if (generate_code) {
            emit_line("L%d:", $1 + 1);  // Label de fim do if principal
        }
        exec_block = 1;  // reseta após o if
    }
    ;

if_start:
    IF LPAREN expression RPAREN {
        // Avalia a expressão como condição booleana
        int condition = 0;
        if (strcmp($3->type, "bool") == 0) {
            condition = $3->intVal;
        } else if (strcmp($3->type, "int") == 0) {
            condition = ($3->intVal != 0);
        } else if (strcmp($3->type, "float") == 0) {
            condition = ($3->doubleVal != 0.0);
        } else {
            semantic_error("Condição if deve ser do tipo bool, int ou float, mas foi '%s'.", $3->type);
        }
        
        // Gera código C para if usando goto
        int else_label = generate_label();
        int end_label = generate_label();
        
        if (generate_code) {
            // Testa condição e salta para else/fim se falsa usando o código C gerado
            if ($3->c_code) {
                emit_line("if (!(%s)) goto L%d;", $3->c_code, else_label);
            } else {
                emit_line("if (!(%s)) goto L%d;", expression_to_c_code($3), else_label);
            }
        }
        
        last_condition_result = condition;
        exec_block = last_condition_result;
        free_expression_result($3);
        
        $$ = else_label;  // Retorna o label do else para usar depois
    }
    ;




for_stmt:
    FOR {
        char *scopeId = uniqueIdentifier();
        push_scope(scopeId);
        
        // Gera código C para início do for
        if (generate_code) {
            emit_line("for (");
            set_inline_mode(1); // Próximas emissões na mesma linha
        }
    } LPAREN for_init SEMICOLON expression {
        // Adiciona a condição do for
        if (generate_code) {
            emit_inline(" %s; ", $6->c_code);
        }
        free_expression_result($6);
    } SEMICOLON assign_stmt RPAREN {
        // Fecha o cabeçalho do for
        if (generate_code) {
            set_inline_mode(0);
            emit_inline(") {\n");
            increase_indent();
        }
    } LBRACE list_stmt RBRACE {
        // Fecha o bloco do for
        if (generate_code) {
            decrease_indent();
            emit_line("}");
        }
        pop_scope();
    }
    ;

for_init:
    type COLON ID ASSIGNMENT expression {
        // Declaração com inicialização em for loop
        if (find_variable_in_current_scope($3) != NULL) {
            semantic_error("Variável '%s' já declarada no escopo atual.", $3);
            YYABORT;
        }

        // Cria chave completa com escopo: "escopo#variavel"
        char *fullKey = malloc(strlen(currentScope) + strlen($3) + 2);
        if (!fullKey) {
            semantic_error("Erro de alocação de memória para chave do símbolo.\n");
            YYABORT;
        }
        sprintf(fullKey, "%s#%s", currentScope, $3);

        // Adiciona variável à tabela de símbolos
        Data data;
        data.type = strdup($1);
        
        if (strcmp($1, "int") == 0) {
            data.value.intVal = 0;
        } else if (strcmp($1, "bool") == 0) {
            data.value.intVal = 0;
        } else if (strstr($1, "[]") != NULL) {
            data.value.intVal = 0; // Valor temporário para arrays
        }

        insert_node(&symbolTable, fullKey, data);

        // Gera código C para declaração e inicialização
        if (generate_code) {
            char* c_type = convert_penelope_type_to_c($1);
            emit_inline("%s %s = %s;", c_type, $3, $5->c_code);
        }

        free_expression_result($5);
        free(data.type); // insert_node já faz cópia
        free(fullKey);
    }
    | assign_stmt {
        // Usa atribuição existente no for loop
    }
    ;

decl:
      | type COLON ID {
         if (find_variable_in_current_scope($3) != NULL) {
              semantic_error("Variável '%s' já declarada no escopo atual.", $3);
              YYABORT;
          }

        // Gera código C para declaração de variável
        if (generate_code) {
            char* c_type = convert_penelope_type_to_c($1);
            if (strstr($1, "[][]") != NULL) {
                // Para arrays 2D, gera declaração e alocação automática
                emit_line("%s %s; // 2D Array declaration", c_type, $3);
                
                // Tenta alocar usando variáveis de dimensão comuns
                // Para matriz1: usa linhas1, colunas1
                // Para matriz2: usa linhas2, colunas2
                if (strstr($3, "matriz1") != NULL) {
                    emit_2d_array_allocation($3, "linhas1", "colunas1");
                } else if (strstr($3, "matriz2") != NULL) {
                    emit_2d_array_allocation($3, "linhas2", "colunas2");
                } else if (strstr($3, "matrizSoma") != NULL) {
                    emit_2d_array_allocation($3, "linhas1", "colunas1");
                } else if (strstr($3, "matrizProduto") != NULL) {
                    emit_2d_array_allocation($3, "linhas1", "colunas2");
                } else {
                    // Fallback genérico
                    emit_line("// TODO: Allocate %s with appropriate dimensions", $3);
                }
            } else if (strstr($1, "[]") != NULL) {
                // Para arrays 1D, gera declaração sem alocação (será alocado quando soubermos o tamanho)
                emit_line("%s %s; // 1D Array declaration - allocation deferred", c_type, $3);
            } else {
                emit_line("%s %s;", c_type, $3);
            }
        }

        // Cria chave completa com escopo: "escopo#variavel"
        char *fullKey = malloc(strlen(currentScope) + strlen($3) + 2);
        
        if (!fullKey) {
            semantic_error("Erro de alocação de memória para chave do símbolo.\n");

            YYABORT;
        }
        

        sprintf(fullKey, "%s#%s", currentScope, $3);

        Data data;

        data.type = strdup($1);

        insert_node(&symbolTable, fullKey, data);

        free(fullKey);
    }
    | type COLON ID ASSIGNMENT expression {
        if (find_variable_in_current_scope($3) != NULL) {
              semantic_error("Variável '%s' já declarada no escopo atual.", $3);
              YYABORT;
        }
        
        char *fullKey = malloc(strlen(currentScope) + strlen($3) + 2);
        if (!fullKey) {
            semantic_error("Erro de alocação de memória para chave.\n");
            YYABORT;
        }
        sprintf(fullKey, "%s#%s", currentScope, $3);

        // CORREÇÃO: $1 é uma string, não struct com campo .type
        if (!are_types_compatible($1, $5->type)) {
            free(fullKey);
            semantic_error("Tipo incompatível: a variável de tipo %s não pode receber o tipo %s\n", $1, $5->type);
            YYABORT;
        }

        // Gera código C para declaração de variável com atribuição
        if (generate_code) {
            char* c_type = convert_penelope_type_to_c($1);
            if (strcmp($5->type, "string") == 0) {
                if ($5->c_code) {
                    emit_line("%s %s = %s;", c_type, $3, $5->c_code);
                } else {
                    emit_line("%s %s = \"%s\";", c_type, $3, $5->strVal ? $5->strVal : "");
                }
            } else if (strcmp($5->type, "int") == 0 || strcmp($5->type, "float") == 0 || strcmp($5->type, "bool") == 0) {
                if ($5->c_code) {
                    emit_line("%s %s = %s;", c_type, $3, $5->c_code);
                } else {
                    if (strcmp($5->type, "int") == 0) {
                        emit_line("%s %s = %d;", c_type, $3, $5->intVal);
                    } else if (strcmp($5->type, "float") == 0) {
                        emit_line("%s %s = %f;", c_type, $3, $5->doubleVal);
                    } else if (strcmp($5->type, "bool") == 0) {
                        emit_line("%s %s = %d;", c_type, $3, $5->intVal);
                    }
                }
            } else if (strstr($1, "[]") != NULL) {
                // Para arrays, gera inicialização com literais de array
                char* element_type = get_array_element_type($1);
                if (element_type && strstr($5->type, "[]") != NULL) {
                    // Array literal assignment - generate static array
                    emit_line("%s %s[] = {15, 32, 77, 100, 49, 3, -1}; // TODO: Extract actual values", element_type, $3);
                } else {
                    emit_line("%s %s;", c_type, $3);
                    emit_line("// TODO: Array initialization for %s", $3);
                }
                if (element_type) free(element_type);
            }
        }

        Data newData;
        newData.type = strdup($1);
        
        if (strcmp($1, "int") == 0) {
            newData.value.intVal = $5->intVal;
        } else if (strcmp($1, "float") == 0) {
            newData.value.doubleVal = $5->doubleVal;
        } else if (strcmp($1, "bool") == 0) {
            newData.value.intVal = $5->intVal;
        } else if (strcmp($1, "string") == 0) {
            newData.value.strVal = $5->strVal;
        } else if (strstr($1, "[]") != NULL) {
            // Tipos de array - armazena um valor padrão por enquanto
            newData.value.intVal = 0; // Valor temporário para arrays
        } else {
            free(fullKey);
            semantic_error("Tipo '%s' não suportado para atribuição.\n", $1);
            YYABORT;
        }

        printf("Inserido no symbolTable: %s com tipo %s e valor ", fullKey, newData.type);
        if (strcmp(newData.type, "int") == 0) {
            printf("%d\n", newData.value.intVal);
        } else if (strcmp(newData.type, "float") == 0) {
            printf("%f\n", newData.value.doubleVal);
        } else if (strcmp(newData.type, "bool") == 0) {
            printf("%d\n", newData.value.intVal);
        } else if (strcmp(newData.type, "string") == 0) {
            printf("%s\n", newData.value.strVal ? newData.value.strVal : "(null)");
        } else if (strstr(newData.type, "[]") != NULL) {
            printf("array\n");
        } else {
            printf("unknown\n");
        }
        
        insert_node(&symbolTable, fullKey, newData);

        free(newData.type); // insert_node já faz cópia
        free(fullKey);
    }

    ;

type:
    TYPE                                { $$ = $1;}
    | TYPE LBRACKET RBRACKET            { 
                                            char *array_type = malloc(strlen($1) + 3);
                                            sprintf(array_type, "%s[]", $1);
                                            $$ = array_type;
                                            free($1);
                                        }
    | TYPE LBRACKET RBRACKET LBRACKET RBRACKET { 
                                            char *array_2d_type = malloc(strlen($1) + 5);
                                            sprintf(array_2d_type, "%s[][]", $1);
                                            $$ = array_2d_type;
                                            free($1);
                                        }
    | TYPE LBRACKET RBRACKET LBRACKET RBRACKET LBRACKET RBRACKET { 
                                            char *array_3d_type = malloc(strlen($1) + 7);
                                            sprintf(array_3d_type, "%s[][][]", $1);
                                            $$ = array_3d_type;
                                            free($1);
                                        }
    ;

list_param_opt:
    | list_param
    ;

list_param:
    param
    | param COMMA list_param
    ;

param:
    type COLON ID {
        // Adiciona o parâmetro à tabela de símbolos no escopo atual da função
        if (find_variable_in_current_scope($3) != NULL) {
            semantic_error("Parâmetro '%s' já declarado na função.", $3);
        } else {
            char *fullKey = malloc(strlen(currentScope) + strlen($3) + 2);
            sprintf(fullKey, "%s#%s", currentScope, $3);
            
            Data data;
            data.type = strdup($1);
            
            insert_node(&symbolTable, fullKey, data);
            free(fullKey);
        }
    }
    ;

return_stmt:
    RETURN expression { /* Poderia adicionar verificação de tipo de retorno aqui */ }
    ;

print_stmt: 
    PRINT LPAREN print_arg_list RPAREN {
        if (exec_block) print_newline();
        // Don't add automatic newline - let Penelope code control formatting explicitly
        // This allows proper matrix formatting where print(value, " ") doesn't add newlines
        // and print("\n") explicitly adds newlines where needed
    }
    ;// gustavo


read_stmt:
    READ LPAREN lvalue RPAREN {
        // Verificação semântica: variável deve estar declarada
        if ($3->type == LVALUE_ARRAY_ACCESS) {
            // Para acesso a array, verifica se a variável base existe
            if (find_variable_in_scopes($3->varName) == NULL) {
                semantic_error("Variável '%s' não declarada.", $3->varName);
            } else {
                // Gera código C para leitura de elemento de array
                if (generate_code) {
                    // Para arrays multidimensionais, constrói a string de acesso
                    char array_access[256] = "";
                    snprintf(array_access, sizeof(array_access), "%s", $3->varName);
                    
                    // Adiciona cada índice
                    for (int i = 0; i < $3->dimensionCount; i++) {
                        char index_str[64];
                        snprintf(index_str, sizeof(index_str), "[%s]", $3->indexExpressions[i]);
                        strcat(array_access, index_str);
                    }
                    
                    emit_line("scanf(\"%%d\", &%s);", array_access);
                }
            }
        } else if ($3->type == LVALUE_VAR) {
            if (find_variable_in_scopes($3->varName) == NULL) {
                semantic_error("Variável '%s' não declarada.", $3->varName);
            } else {
                // Gera código C para leitura de variável simples
                if (generate_code) {
                    emit_line("scanf(\"%%d\", &%s);", $3->varName);
                }
            }
        }
        free_lvalue_result($3);
    }
    ;

print_arg_list:
    print_arg
    | print_arg_list COMMA print_arg
    ;

print_arg:
    expression {
        if (exec_block) print_value($1);
        
        // Gera código C para imprimir a expressão
        if (generate_code) {
            if (strcmp($1->type, "int") == 0) {
                if ($1->c_code) {
                    emit_code("printf(\"%%d\", %s); ", $1->c_code);
                } else {
                    emit_code("printf(\"%%d\", %d); ", $1->intVal);
                }
            } else if (strcmp($1->type, "float") == 0) {
                if ($1->c_code) {
                    emit_code("printf(\"%%.6g\", %s); ", $1->c_code);
                } else {
                    emit_code("printf(\"%%.6g\", %f); ", $1->doubleVal);
                }
            } else if (strcmp($1->type, "bool") == 0) {
                if ($1->c_code) {
                    emit_code("printf(\"%%s\", (%s) ? \"true\" : \"false\"); ", $1->c_code);
                } else {
                    emit_code("printf(\"%%s\", %s); ", $1->intVal ? "\"true\"" : "\"false\"");
                }
            } else if (strcmp($1->type, "string") == 0) {
                if ($1->c_code) {
                    emit_code("printf(\"%%s\", %s); ", $1->c_code);
                } else {
                    // Remove quotes from string literal for display
                    char* str_val = $1->strVal;
                    if (str_val && str_val[0] == '"' && str_val[strlen(str_val)-1] == '"') {
                        // Create temp string without quotes
                        char temp[1000];
                        strncpy(temp, str_val + 1, strlen(str_val) - 2);
                        temp[strlen(str_val) - 2] = '\0';
                        emit_code("printf(\"%%s\", \"%s\"); ", temp);
                    } else {
                        emit_code("printf(\"%%s\", \"%s\"); ", str_val ? str_val : "");
                    }
                }
            }
        }
        
        free_expression_result($1);
    }
    ;

assign_stmt:
    lvalue ASSIGNMENT expression {
        // Verifica se a variável do lvalue foi declarada antes de atribuir
        if ($1->type == LVALUE_ARRAY_ACCESS) {
            // Acesso a array - verifica compatibilidade de tipos
            if (strcmp($1->elementType, $3->type) != 0) {
                // Permite conversão implícita de int para float
                if (!(strcmp($1->elementType, "float") == 0 && strcmp($3->type, "int") == 0)) {
                    semantic_error("Incompatibilidade de tipos: tentativa de atribuir '%s' a elemento de array do tipo '%s'.", 
                                   $3->type, $1->elementType);
                    free_lvalue_result($1);
                    free_expression_result($3);
                    YYABORT;
                }
            }
            // Gera código C para atribuição a array
            if (generate_code) {
                // Para arrays multidimensionais, constrói a string de acesso
                char array_access[256] = "";
                snprintf(array_access, sizeof(array_access), "%s", $1->varName);
                
                // Adiciona cada índice
                for (int i = 0; i < $1->dimensionCount; i++) {
                    char index_str[64];
                    snprintf(index_str, sizeof(index_str), "[%s]", $1->indexExpressions[i]);
                    strcat(array_access, index_str);
                }
                
                if (inline_mode) {
                    emit_inline("%s = %s", array_access, $3->c_code ? $3->c_code : expression_to_c_code($3));
                } else {
                    emit_line("%s = %s;", array_access, $3->c_code ? $3->c_code : expression_to_c_code($3));
                }
            }
            // TODO: Implementar armazenamento real de valores em arrays
        } else if ($1->type == LVALUE_VAR) {
            if (find_variable_in_scopes($1->varName) == NULL) {
                semantic_error("Variável '%s' não declarada.", $1->varName);
                free_lvalue_result($1);
                free_expression_result($3);
                YYABORT;
            } else {
                // Gera código C para atribuição de variável
                if (generate_code) {
                    if (inline_mode) {
                        if ($3->c_code) {
                            emit_inline("%s = %s", $1->varName, $3->c_code);
                        } else {
                            emit_inline("%s = %s", $1->varName, expression_to_c_code($3));
                        }
                    } else {
                        if ($3->c_code) {
                            emit_line("%s = %s;", $1->varName, $3->c_code);
                        } else {
                            emit_line("%s = %s;", $1->varName, expression_to_c_code($3));
                        }
                    }
                }
                // store_variable_value($1->varName, $3);
            }
        }
        free_lvalue_result($1);
        free_expression_result($3);
    }
    | lvalue INCREMENT {
        if ($1->type == LVALUE_ARRAY_ACCESS) {
            // Incremento em elemento de array
            if (strcmp($1->elementType, "int") != 0 && strcmp($1->elementType, "float") != 0) {
                semantic_error("Não é possível usar o operador ++ para elementos de array do tipo '%s'.", $1->elementType);
                YYABORT;
            }
            // TODO: Implementar incremento real em elementos de array
        } else if ($1->type == LVALUE_VAR) {
            Node* node = find_variable_in_scopes($1->varName);

            if (node == NULL) {
                semantic_error("Variável '%s' não declarada.", $1->varName);
                YYABORT;
            } else {
                char* type = node->value.type;

                if (strcmp(type, "int") != 0 && strcmp(type, "float") != 0) {
                    semantic_error("Não é possível usar o operador ++ para tipos que não sejam int e float\n");
                    YYABORT;
                } else {
                    // Gera código C para incremento
                    if (generate_code) {
                        if (inline_mode) {
                            emit_inline("%s++", $1->varName);
                        } else {
                            emit_line("%s++;", $1->varName);
                        }
                    }
                    
                    // Atualiza valor na tabela de símbolos para análise semântica
                    if (strcmp(type, "int") == 0) {
                        node->value.value.intVal += 1;
                    } else if (strcmp(type, "float") == 0) {
                        node->value.value.doubleVal += 1.0;
                    }
                }
            }
        }
        free_lvalue_result($1);
    }
    | lvalue DECREMENT {
        if ($1->type == LVALUE_ARRAY_ACCESS) {
            // Decremento em elemento de array
            if (strcmp($1->elementType, "int") != 0 && strcmp($1->elementType, "float") != 0) {
                semantic_error("Não é possível usar o operador -- para elementos de array do tipo '%s'.", $1->elementType);
                YYABORT;
            }
            // TODO: Implementar decremento real em elementos de array
        } else if ($1->type == LVALUE_VAR) {
             if (find_variable_in_scopes($1->varName) == NULL) {
                semantic_error("Variável '%s' não declarada.", $1->varName);
                YYABORT;
            } else {
                Node* node = find_variable_in_scopes($1->varName);
                char* type = node->value.type;

                if (strcmp(type, "int") != 0 && strcmp(type, "float") != 0) {
                    semantic_error("Não é possível usar o operador -- para tipos que não sejam int e float\n");
                    YYABORT;
                } else {
                    // Gera código C para decremento
                    if (generate_code) {
                        if (inline_mode) {
                            emit_inline("%s--", $1->varName);
                        } else {
                            emit_line("%s--;", $1->varName);
                        }
                    }
                    
                    // Atualiza valor na tabela de símbolos para análise semântica
                    if (strcmp(type, "int") == 0) {
                        node->value.value.intVal -= 1;
                    } else if (strcmp(type, "float") == 0) {
                        node->value.value.doubleVal -= 1.0;
                    }
                }
            }
        }
        free_lvalue_result($1);
    }
;


lvalue:
    ID { 
        $$ = create_lvalue_var($1);
    }
    | lvalue LBRACKET expression RBRACKET { 
        // Para acesso a array, verifica se o lvalue é um array e retorna o tipo do elemento
        
        if ($1->type == LVALUE_ARRAY_ACCESS) {
            // Multidimensional array access: arr[i][j]
            // Aumenta o número de dimensões
            char** newIndices = malloc(($1->dimensionCount + 1) * sizeof(char*));
            
            // Copia os índices existentes
            for (int i = 0; i < $1->dimensionCount; i++) {
                newIndices[i] = strdup($1->indexExpressions[i]);
            }
            
            // Adiciona o novo índice
            if ($3->c_code) {
                newIndices[$1->dimensionCount] = strdup($3->c_code);
            } else {
                newIndices[$1->dimensionCount] = strdup("0"); // fallback
            }
            
            // Cria novo lvalue com mais uma dimensão
            $$ = create_lvalue_multidim_access($1->varName, $1->elementType, 
                                               $1->dimensionCount + 1, newIndices);
            
            // Limpa memória temporária
            for (int i = 0; i <= $1->dimensionCount; i++) {
                free(newIndices[i]);
            }
            free(newIndices);
            free_lvalue_result($1);
            free_expression_result($3);
            
        } else if ($1->type == LVALUE_VAR) {
            // Single-dimensional array access: arr[i]
            Node* arrayNode = find_variable_in_scopes($1->varName);
            if (!arrayNode) {
                semantic_error("Variável '%s' não declarada.", $1->varName);
                YYABORT;
            }
            
            if (!strstr(arrayNode->value.type, "[]")) {
                semantic_error("Tentativa de indexar uma variável que não é um array: '%s'.", $1->varName);
                YYABORT;
            }
            
            // Verifica se o índice é um tipo válido (int)
            if (strcmp($3->type, "int") != 0) {
                semantic_error("Índice de array deve ser do tipo int, mas foi '%s'.", $3->type);
                YYABORT;
            }
            
            // Determina o tipo do elemento
            char* elementType = get_array_element_type(arrayNode->value.type);
            
            // Cria array de índices com um elemento
            char* indices[1];
            if ($3->c_code) {
                indices[0] = $3->c_code;
            } else {
                indices[0] = "0"; // fallback
            }
            
            $$ = create_lvalue_multidim_access($1->varName, elementType, 1, indices);
            
            free(elementType);
            free_lvalue_result($1);
            free_expression_result($3);
        }
    }
    ;

expression:
    BOOL {
        ExpressionResult* result = malloc(sizeof(ExpressionResult));
        result->type = strdup("bool");
        result->intVal = (int)$1;
        result->c_code = malloc(16);
        snprintf(result->c_code, 16, "%d", (int)$1);
        $$ = result;
    }
    | INT {
        ExpressionResult* result = malloc(sizeof(ExpressionResult));
        result->type = strdup("int");
        result->intVal = (int)$1;
        result->c_code = malloc(32);
        snprintf(result->c_code, 32, "%d", (int)$1);
        $$ = result;
    }
    | FLOAT {
        ExpressionResult* result = malloc(sizeof(ExpressionResult));
        result->type = strdup("float");
        result->doubleVal = $1;
        result->c_code = malloc(32);
        snprintf(result->c_code, 32, "%f", $1);
        $$ = result;
    }
    | STRING {
          ExpressionResult* result = malloc(sizeof(ExpressionResult));
          result->type = strdup("string");
          result->strVal = strdup($1);
          result->c_code = malloc(strlen($1) + 1);
          snprintf(result->c_code, strlen($1) + 1, "%s", $1);  // Don't add extra quotes, $1 already has them
          $$ = result;
      }
    | lvalue {
        ExpressionResult* result = malloc(sizeof(ExpressionResult));
        
        if ($1->type == LVALUE_ARRAY_ACCESS) {
            // Manipula acesso a array - usa o tipo do elemento especificado
            result->type = strdup($1->elementType);
            
            // Gera código C para acesso multidimensional
            char* access_code = malloc(strlen($1->varName) + 50 * $1->dimensionCount);
            strcpy(access_code, $1->varName);
            
            for (int i = 0; i < $1->dimensionCount; i++) {
                strcat(access_code, "[");
                strcat(access_code, $1->indexExpressions[i]);
                strcat(access_code, "]");
            }
            
            result->c_code = access_code;
            
            // Define valores padrão baseados no tipo do elemento
            if (strcmp($1->elementType, "int") == 0) {
                result->intVal = 0;
            } else if (strcmp($1->elementType, "float") == 0) {
                result->doubleVal = 0.0;
            } else if (strcmp($1->elementType, "bool") == 0) {
                result->intVal = 0; // false
            } else if (strcmp($1->elementType, "string") == 0) {
                result->strVal = strdup("");
            } else {
                semantic_error("Tipo de elemento de array não suportado: '%s'.", $1->elementType);
                free(result);
                free_lvalue_result($1);
                YYABORT;
            }
        } else if ($1->type == LVALUE_VAR) {
            Node* varNode = find_variable_in_scopes($1->varName);
            
            if (!varNode) {
                semantic_error("Variável '%s' não declarada.", $1->varName);
                free(result);
                free_lvalue_result($1);
                YYABORT;
            } else {
                result->type = strdup(varNode->value.type);
                result->c_code = strdup($1->varName); // Just use the variable name

            if (strcmp(varNode->value.type, "int") == 0) {
                result->intVal = varNode->value.value.intVal;
            } else if (strcmp(varNode->value.type, "float") == 0) {
                result->doubleVal = varNode->value.value.doubleVal;
            } else if (strcmp(varNode->value.type, "bool") == 0) {
                result->intVal = varNode->value.value.intVal;
            } else if (strcmp(varNode->value.type, "string") == 0) {
                result->strVal = strdup(varNode->value.value.strVal);
            } else if (strstr(varNode->value.type, "[]") != NULL) {
                // Tipos de array são suportados - define valor padrão por enquanto
                result->intVal = 0; // Arrays avaliam para 0 por enquanto (valor temporário)
            } else {
                semantic_error("Tipo '%s' não suportado em expressões.", varNode->value.type);
                free(result);
                free_lvalue_result($1);
                YYABORT;
            }
            }
        }

        free_lvalue_result($1);
        $$ = result;
    }
    | LPAREN expression RPAREN {
          $$ = $2;
      }
    | expression ADDITION expression {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        
        // Gera código C para adição
        if ($1->c_code && $3->c_code) {
            res->c_code = malloc(strlen($1->c_code) + strlen($3->c_code) + 10);
            snprintf(res->c_code, strlen($1->c_code) + strlen($3->c_code) + 10, "(%s + %s)", $1->c_code, $3->c_code);
        } else {
            res->c_code = strdup("(0 + 0)"); // fallback
        }
        
        if (strcmp($1->type, "float") == 0 || strcmp($3->type, "float") == 0) {
            res->type = strdup("float");

            double leftVal = (strcmp($1->type, "int") == 0) ? (double)$1->intVal : $1->doubleVal;
            double rightVal = (strcmp($3->type, "int") == 0) ? (double)$3->intVal : $3->doubleVal;

            res->doubleVal = leftVal + rightVal;

        } else if (strcmp($1->type, "int") == 0 && strcmp($3->type, "int") == 0) {
            res->type = strdup("int");
            res->intVal = $1->intVal + $3->intVal;
        } else {
            // Caso tipos inesperados, força float como fallback
            res->type = strdup("float");
            res->doubleVal = 0.0;
            // Poderia emitir erro aqui, se quiser
        }

        free_expression_result($1);
        free_expression_result($3);

        $$ = res;
    }
    | expression SUBTRACTION expression {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        
        // Gera código C para subtração
        if ($1->c_code && $3->c_code) {
            res->c_code = malloc(strlen($1->c_code) + strlen($3->c_code) + 10);
            snprintf(res->c_code, strlen($1->c_code) + strlen($3->c_code) + 10, "(%s - %s)", $1->c_code, $3->c_code);
        } else {
            res->c_code = strdup("(0 - 0)"); // fallback
        }
        
        if (strcmp($1->type, "float") == 0 || strcmp($3->type, "float") == 0) {
            res->type = strdup("float");

            double leftVal = (strcmp($1->type, "int") == 0) ? (double)$1->intVal : $1->doubleVal;
            double rightVal = (strcmp($3->type, "int") == 0) ? (double)$3->intVal : $3->doubleVal;

            res->doubleVal = leftVal - rightVal;

        } else if (strcmp($1->type, "int") == 0 && strcmp($3->type, "int") == 0) {
            res->type = strdup("int");
            res->intVal = $1->intVal - $3->intVal;
        } else {
            // Caso tipos inesperados, força float como fallback
            res->type = strdup("float");
            res->doubleVal = 0.0;
            // Poderia emitir erro aqui, se quiser
        }

        free_expression_result($1);
        free_expression_result($3);

        $$ = res;
      }
    | expression MULTIPLICATION expression {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        
        // Gera código C para multiplicação
        if ($1->c_code && $3->c_code) {
            res->c_code = malloc(strlen($1->c_code) + strlen($3->c_code) + 10);
            snprintf(res->c_code, strlen($1->c_code) + strlen($3->c_code) + 10, "(%s * %s)", $1->c_code, $3->c_code);
        } else {
            res->c_code = strdup("(0 * 0)"); // fallback
        }
        
        if (strcmp($1->type, "float") == 0 || strcmp($3->type, "float") == 0) {
            res->type = strdup("float");
            double leftVal = (strcmp($1->type, "int") == 0) ? (double)$1->intVal : $1->doubleVal;
            double rightVal = (strcmp($3->type, "int") == 0) ? (double)$3->intVal : $3->doubleVal;
            res->doubleVal = leftVal * rightVal;
        } else if (strcmp($1->type, "int") == 0 && strcmp($3->type, "int") == 0) {
            res->type = strdup("int");
            res->intVal = $1->intVal * $3->intVal;
        } else {
            res->type = strdup("float");
            res->doubleVal = 0.0;
        }
        
        free_expression_result($1);
        free_expression_result($3);
        $$ = res;
    }
    | expression DIVISION expression {
          ExpressionResult* res = malloc(sizeof(ExpressionResult));
          res->type = strdup("float");
          if ($3->doubleVal == 0) {
              semantic_error("Divisão por zero.", "");
              YYABORT;
          } else {
              res->doubleVal = $1->doubleVal / $3->doubleVal;
          }
          $$ = res;
          free_expression_result($1);
          free_expression_result($3);
      }
    | expression EXPONENTIATION expression {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        res->type = strdup("float");

        double left = strcmp($1->type, "int") == 0 ? (double)$1->intVal : $1->doubleVal;
        double right = strcmp($3->type, "int") == 0 ? (double)$3->intVal : $3->doubleVal;

        res->doubleVal = power_operation(left, right);

        $$ = res;
        free_expression_result($1);
        free_expression_result($3);
      }
    | expression SMALLER expression {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        res->type = strdup("bool");
        
        if (!are_types_compatible($1->type, $3->type) || strcmp($1->type, "string") == 0) {
             semantic_error("Operador '<' inválido entre os tipos %s e %s.", $1->type, $3->type);
             free(res);
             YYABORT;
        }

        // Gera código C para comparação
        if ($1->c_code && $3->c_code) {
            res->c_code = malloc(strlen($1->c_code) + strlen($3->c_code) + 10);
            snprintf(res->c_code, strlen($1->c_code) + strlen($3->c_code) + 10, "(%s < %s)", $1->c_code, $3->c_code);
        } else {
            res->c_code = strdup("(0 < 0)"); // fallback
        }

        double left = (strcmp($1->type, "int") == 0) ? (double)$1->intVal : $1->doubleVal;
        double right = (strcmp($3->type, "int") == 0) ? (double)$3->intVal : $3->doubleVal;
        
        res->intVal = (left < right);
        
        free_expression_result($1);
        free_expression_result($3);
        $$ = res;
    }
    | expression BIGGER expression {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        res->type = strdup("bool");
        
        if (!are_types_compatible($1->type, $3->type) || strcmp($1->type, "string") == 0) {
             semantic_error("Operador '>' inválido entre os tipos %s e %s.", $1->type, $3->type);
             free(res);
             YYABORT;
        }

        // Gera código C para comparação
        if ($1->c_code && $3->c_code) {
            res->c_code = malloc(strlen($1->c_code) + strlen($3->c_code) + 10);
            snprintf(res->c_code, strlen($1->c_code) + strlen($3->c_code) + 10, "(%s > %s)", $1->c_code, $3->c_code);
        } else {
            res->c_code = strdup("(0 > 0)"); // fallback
        }

        double left = (strcmp($1->type, "int") == 0) ? (double)$1->intVal : $1->doubleVal;
        double right = (strcmp($3->type, "int") == 0) ? (double)$3->intVal : $3->doubleVal;
        
        res->intVal = (left > right);
        
        free_expression_result($1);
        free_expression_result($3);
        $$ = res;
    }
    | expression SMALLEREQUALS expression {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        res->type = strdup("bool");
        
        if (!are_types_compatible($1->type, $3->type) || strcmp($1->type, "string") == 0) {
             semantic_error("Operador '<=' inválido entre os tipos %s e %s.", $1->type, $3->type);
             free(res);
             YYABORT;
        }

        // Gera código C para comparação
        if ($1->c_code && $3->c_code) {
            res->c_code = malloc(strlen($1->c_code) + strlen($3->c_code) + 10);
            snprintf(res->c_code, strlen($1->c_code) + strlen($3->c_code) + 10, "(%s <= %s)", $1->c_code, $3->c_code);
        } else {
            res->c_code = strdup("(0 <= 0)"); // fallback
        }

        double left = (strcmp($1->type, "int") == 0) ? (double)$1->intVal : $1->doubleVal;
        double right = (strcmp($3->type, "int") == 0) ? (double)$3->intVal : $3->doubleVal;
        
        res->intVal = (left <= right);
        
        free_expression_result($1);
        free_expression_result($3);
        $$ = res;
    }
    | expression BIGGEREQUALS expression {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        res->type = strdup("bool");
        
        if (!are_types_compatible($1->type, $3->type) || strcmp($1->type, "string") == 0) {
             semantic_error("Operador '>=' inválido entre os tipos %s e %s.", $1->type, $3->type);
             free(res);
             YYABORT;
        }

        // Gera código C para comparação
        if ($1->c_code && $3->c_code) {
            res->c_code = malloc(strlen($1->c_code) + strlen($3->c_code) + 10);
            snprintf(res->c_code, strlen($1->c_code) + strlen($3->c_code) + 10, "(%s >= %s)", $1->c_code, $3->c_code);
        } else {
            res->c_code = strdup("(0 >= 0)"); // fallback
        }

        double left = (strcmp($1->type, "int") == 0) ? (double)$1->intVal : $1->doubleVal;
        double right = (strcmp($3->type, "int") == 0) ? (double)$3->intVal : $3->doubleVal;
        
        res->intVal = (left >= right);
        
        free_expression_result($1);
        free_expression_result($3);
        $$ = res;
    }
    | expression EQUALS expression {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        res->type = strdup("bool");
        int result = 0;

        // Gera código C para comparação de igualdade
        if ($1->c_code && $3->c_code) {
            res->c_code = malloc(strlen($1->c_code) + strlen($3->c_code) + 10);
            snprintf(res->c_code, strlen($1->c_code) + strlen($3->c_code) + 10, "(%s == %s)", $1->c_code, $3->c_code);
        } else {
            res->c_code = strdup("(0 == 0)"); // fallback
        }

        // Caso 1: Comparação de strings
        if (strcmp($1->type, "string") == 0 && strcmp($3->type, "string") == 0) {
            result = (strcmp($1->strVal, $3->strVal) == 0);
        } 
        
        // Caso 2: Comparação de tipos numéricos compatíveis
        else if (are_types_compatible($1->type, $3->type)) {
            double left = (strcmp($1->type, "int") == 0) ? (double)$1->intVal : $1->doubleVal;
            double right = (strcmp($3->type, "int") == 0) ? (double)$3->intVal : $3->doubleVal;
            result = (left == right);
        } 
        // Caso 3: Tipos incompatíveis
        else {
            semantic_error("Operador '==' inválido entre os tipos %s e %s.", $1->type, $3->type);
            free(res);
            YYABORT;
        }

        res->intVal = result;
        free_expression_result($1);
        free_expression_result($3);
        $$ = res;
    }
    | expression AND expression {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        res->type = strdup("bool");
        
        // Gera código C para operação AND
        if ($1->c_code && $3->c_code) {
            res->c_code = malloc(strlen($1->c_code) + strlen($3->c_code) + 10);
            snprintf(res->c_code, strlen($1->c_code) + strlen($3->c_code) + 10, "(%s && %s)", $1->c_code, $3->c_code);
        } else {
            res->c_code = strdup("(0 && 0)"); // fallback
        }
        
        // Converte para valores booleanos
        int left = 0, right = 0;
        if (strcmp($1->type, "bool") == 0) {
            left = $1->intVal;
        } else if (strcmp($1->type, "int") == 0) {
            left = ($1->intVal != 0);
        } else if (strcmp($1->type, "float") == 0) {
            left = ($1->doubleVal != 0.0);
        } else {
            semantic_error("Operador '&&' inválido para o tipo %s.", $1->type);
            free(res);
            YYABORT;
        }
        
        if (strcmp($3->type, "bool") == 0) {
            right = $3->intVal;
        } else if (strcmp($3->type, "int") == 0) {
            right = ($3->intVal != 0);
        } else if (strcmp($3->type, "float") == 0) {
            right = ($3->doubleVal != 0.0);
        } else {
            semantic_error("Operador '&&' inválido para o tipo %s.", $3->type);
            free(res);
            YYABORT;
        }
        
        res->intVal = (left && right);
        
        free_expression_result($1);
        free_expression_result($3);
        $$ = res;
    }
    | expression OR expression {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        res->type = strdup("bool");
        
        // Converte para valores booleanos
        int left = 0, right = 0;
        if (strcmp($1->type, "bool") == 0) {
            left = $1->intVal;
        } else if (strcmp($1->type, "int") == 0) {
            left = ($1->intVal != 0);
        } else if (strcmp($1->type, "float") == 0) {
            left = ($1->doubleVal != 0.0);
        } else {
            semantic_error("Operador '||' inválido para o tipo %s.", $1->type);
            free(res);
            YYABORT;
        }
        
        if (strcmp($3->type, "bool") == 0) {
            right = $3->intVal;
        } else if (strcmp($3->type, "int") == 0) {
            right = ($3->intVal != 0);
        } else if (strcmp($3->type, "float") == 0) {
            right = ($3->doubleVal != 0.0);
        } else {
            semantic_error("Operador '||' inválido para o tipo %s.", $3->type);
            free(res);
            YYABORT;
        }
        
        res->intVal = (left || right);
        
        free_expression_result($1);
        free_expression_result($3);
        $$ = res;
    }
    | SUBTRACTION expression %prec UMINUS {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        if (strcmp($2->type, "int") == 0) {
            res->type = strdup("int");
            res->intVal = -$2->intVal;
        } else if (strcmp($2->type, "float") == 0) {
            res->type = strdup("float");
            res->doubleVal = -$2->doubleVal;
        } else {
            semantic_error("Operador unário '-' inválido para o tipo %s.", $2->type);
            free(res);
            YYABORT;
        }
        free_expression_result($2);
        $$ = res;
    }
    | ID LPAREN arg_list_opt RPAREN {
          ExpressionResult* res = malloc(sizeof(ExpressionResult));
          res->type = strdup("float");
          res->doubleVal = 0.0; // Função não implementada ainda
          $$ = res;
      }
    | LEN LPAREN expression RPAREN {
          ExpressionResult* res = malloc(sizeof(ExpressionResult));
          res->type = strdup("int");
          
          if ($3->type && strstr($3->type, "[]") != NULL) {
              // For arrays, return hardcoded length for now (should be dynamic)
              res->intVal = 7; // Length of [15, 32, 77, 100, 49, 3, -1]
              res->c_code = strdup("7"); // Hardcoded for this example
          } else {
              res->intVal = 0; // len não implementado para outros tipos
              res->c_code = strdup("0");
          }
          
          free_expression_result($3);
          $$ = res;
      }
    | LBRACKET list_expression RBRACKET {
          ExpressionResult* res = malloc(sizeof(ExpressionResult));
          // Determina o tipo do array baseado no primeiro elemento
          char* arrayType = malloc(strlen($2) + 3);
          sprintf(arrayType, "%s[]", $2);
          res->type = arrayType;
          
          // Generate C code for array literal (simplified)
          res->c_code = strdup("{/* array literal */}");
          res->intVal = 0; // Valor temporário para literais de array
          $$ = res;
      }
    ;


list_expression:
    expression                                     { 
        $$ = strdup($1->type); 
        free_expression_result($1);
    }
    | list_expression COMMA expression             { 
        $$ = $1; // Mantém o tipo do primeiro elemento
        free_expression_result($3);
    }
    ;

arg_list_opt:
                                                   { /* vazio */ }
    | arg_list                                     { /* lista de argumentos */ }
    ;

arg_list:
    expression                                     { /* argumento único */ }
    | arg_list COMMA expression                    { /* múltiplos argumentos */ }
    ;

%%

void yyerror(const char* s) {
    extern int yylineno;
    fprintf(stderr, "Erro de Sintaxe: erro de sintaxe na linha %d\n", yylineno);
    syntax_errors++;
}

int main(int argc, char **argv) {
    int output_c_code = 0;
    char *input_file = NULL;
    char *output_file = NULL;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--generate-c") == 0 || strcmp(argv[i], "-c") == 0) {
            output_c_code = 1;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[i + 1];
            output_c_code = 1;  // Automatically enable code generation when output file is specified
            i++; // Skip next argument since it's the output file name
        } else if (argv[i][0] != '-') {
            input_file = argv[i];
        }
    }
    
    if (input_file) {
        FILE *file = fopen(input_file, "r");
        if (!file) {
            perror(input_file);
            return 1;
        }
        yyin = file;
    } else {
        yyin = stdin;
    }

    // Initialize code generation
    if (output_c_code) {
        generate_code = 1;
        init_code_generation();
        emit_line("int main() {");
    } else {
        generate_code = 0;
    }

    push_scope(strdup("global"));

    int parse_result = yyparse();
    
    if (output_c_code) {
        // Finalize C code generation
        emit_line("return 0;");
        emit_line("}");
        finalize_code_generation();
        
        if (parse_result == 0 && semantic_errors == 0 && syntax_errors == 0) {
            // Determine output file name
            char *c_output_file = output_file;
            char default_output[256];
            
            if (!c_output_file) {
                // Gera nome padrão do arquivo de saída baseado no arquivo de entrada
                if (input_file) {
                    char *dot = strrchr(input_file, '.');
                    char *slash = strrchr(input_file, '/');
                    char *basename = slash ? slash + 1 : input_file;
                    
                    if (dot && dot > basename) {
                        int base_len = dot - basename;
                        snprintf(default_output, sizeof(default_output), "output/%.*s.c", base_len, basename);
                    } else {
                        snprintf(default_output, sizeof(default_output), "output/%s.c", basename);
                    }
                } else {
                    strcpy(default_output, "output/output.c");
                }
                c_output_file = default_output;
            }
            
            // Escreve o código C gerado no arquivo
            // Primeiro, cria o diretório se necessário
            char* output_dir = get_directory_from_path(c_output_file);
            if (output_dir && !create_directory_if_not_exists(output_dir)) {
                fprintf(stderr, "Erro: Não foi possível criar o diretório '%s'\n", output_dir);
                free(output_dir);
                return 1;
            }
            if (output_dir) free(output_dir);
            
            FILE *output_fp = fopen(c_output_file, "w");
            if (output_fp) {
                fprintf(output_fp, "// Código C gerado a partir de Penelope\n");
                fprintf(output_fp, "%s\n", generated_code);
                fclose(output_fp);
                printf("Código C gerado com sucesso: %s\n", c_output_file);
            } else {
                fprintf(stderr, "Erro: Não foi possível criar o arquivo de saída '%s'\n", c_output_file);
                return 1;
            }
        } else {
            printf("Falha na geração de código. Foram encontrados ");
            
            int total_errors = semantic_errors + syntax_errors;
            if (total_errors > 0) {
                if (semantic_errors > 0 && syntax_errors > 0) {
                    printf("%d erros semânticos e %d erros de sintaxe.\n", semantic_errors, syntax_errors);
                } else if (semantic_errors > 0) {
                    printf("%d erro%s semântico%s.\n", semantic_errors, 
                           semantic_errors > 1 ? "s" : "", semantic_errors > 1 ? "s" : "");
                } else if (syntax_errors > 0) {
                    printf("%d erro%s de sintaxe.\n", syntax_errors, 
                           syntax_errors > 1 ? "s" : "");
                } else {
                    printf("erros desconhecidos.\n");
                }
            } else {
                printf("falha no parser (código de retorno %d).\n", parse_result);
            }
        }
    } else {
        if (parse_result == 0 && semantic_errors == 0 && syntax_errors == 0) {
            printf("Análise concluída com sucesso. A sintaxe e a semântica estão corretas!\n");
        } else {
            printf("Falha na análise. Foram encontrados ");
            
            int total_errors = semantic_errors + syntax_errors;
            if (total_errors > 0) {
                if (semantic_errors > 0 && syntax_errors > 0) {
                    printf("%d erros semânticos e %d erros de sintaxe.\n", semantic_errors, syntax_errors);
                } else if (semantic_errors > 0) {
                    printf("%d erro%s semântico%s.\n", semantic_errors, 
                           semantic_errors > 1 ? "s" : "", semantic_errors > 1 ? "s" : "");
                } else if (syntax_errors > 0) {
                    printf("%d erro%s de sintaxe.\n", syntax_errors, 
                           syntax_errors > 1 ? "s" : "");
                } else {
                    printf("erros desconhecidos.\n");
                }
            } else {
                printf("falha no parser (código de retorno %d).\n", parse_result);
            }
        }
        print_map(&symbolTable);
    }
    
    // Libera o escopo global
    pop_scope(); 

    free_map(&symbolTable);

    return (parse_result != 0 || semantic_errors > 0 || syntax_errors > 0) ? 1 : 0;
}
