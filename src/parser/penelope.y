%{
#include <stddef.h>
#include "../src/parser/grammarActions/grammarActions.h"
#include "../src/parser/codeGenerator/codeGenerator.h"
#include "../src/parser/semantics/semantics.h"
#include "../src/parser/symbolTable/symbolTable.h"
#include "../src/parser/expressionUtils/expressionUtils.h"
#include "../src/utils/uniqueIdentifier/uniqueIdentifier.h"
#include "../src/structs/lvalue/lvalueResult.h"

extern int yylineno;
extern int yylex();
extern void yyerror(const char* s);

// Coleta de parâmetros para assinaturas de função
typedef struct FunctionParam {
    char* type;
    char* name;
    struct FunctionParam* next;
} FunctionParam;

FunctionParam* current_function_params = NULL;
char* current_function_name = NULL;
char* current_function_return_type = NULL;

// Coleta de argumentos para chamadas de função
typedef struct FunctionArg {
    char* code;
    struct FunctionArg* next;
} FunctionArg;

FunctionArg* current_function_args = NULL;

// Análise de definição de structs
char* current_struct_name = NULL;
StructField* current_struct_fields = NULL;

// Declarações antecipadas para funções auxiliares
void add_function_parameter(const char* type, const char* name);
void clear_function_parameters();
void clear_function_context();
void emit_function_signature();
void emit_function_signature_with_info(const char* return_type, const char* function_name);
void add_function_argument(const char* arg_code);
void clear_function_arguments();
char* get_function_arguments();

%}

/* Inclua o header aqui, *fora* do bloco %{...%}, para que o Bison leia a definição do tipo ANTES do %union */
%code requires {
    #include <string.h>
    #include "../src/structs/expression/expressionResult.h"
    #include "../src/structs/lvalue/lvalueResult.h"
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
%token MODULO


%token <num> INT FLOAT

%token FUN STRUCT WHILE FOR IF ELSE LEN PRINT READ RETURN
%token LBRACKET RBRACKET COMMA LPAREN RPAREN COLON SEMICOLON NEWLINE DOT
%token ASSIGNMENT EQUALS SMALLEREQUALS BIGGEREQUALS SMALLER BIGGER
%token INCREMENT DECREMENT EXPONENTIATION MULTIPLICATION DIVISION ADDITION SUBTRACTION
%token LBRACE RBRACE

%define parse.trace

%type <lvalueResult> lvalue
%type <str> type array_values
%type <exprResult> expression
%type <labelNum> while_start if_start

%left OR
%left AND
%left EQUALS
%left SMALLEREQUALS BIGGEREQUALS SMALLER BIGGER
%left ADDITION SUBTRACTION
%left MULTIPLICATION DIVISION MODULO
%right EXPONENTIATION   
%nonassoc UMINUS 
%right ASSIGNMENT
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
    | struct_def
    | decl SEMICOLON
    | NEWLINE
    ;

fun:
    FUN type ID LPAREN {
        // Armazena informações da função para coleta de parâmetros
        current_function_name = strdup($3);
        current_function_return_type = strdup($2);
        clear_function_parameters(); // Limpa parâmetros anteriores
        
        // Armazena informações da função na tabela de símbolos
        store_function($3, $2);
        
        // Sempre habilita geração de código para todas as funções
        enable_code_generation();
        
        // Verifica se é a função main
        if (strcmp($3, "main") == 0) {
            in_main_function = 1;
        } else {
            in_main_function = 0;
        }
        
        // Cria o escopo da função antes de processar parâmetros
        char *scopeId = uniqueIdentifier();
        push_scope(scopeId);
    } list_param_opt RPAREN LBRACE {
        // Agora emite a assinatura da função com todos os parâmetros coletados
        // Passa as informações da função diretamente em vez de usar variáveis globais
        emit_function_signature_with_info($2, $3);
    } list_stmt RBRACE {
        // Remove o escopo da função
        pop_scope();
        
        // Fecha a função
        if (generate_code) {
            if (in_main_function) {
                emit_line("return 0;");
            }
            decrease_indent();
            emit_line("}");
        }
        
        // Limpa parâmetros e contexto da função
        clear_function_parameters();
        clear_function_context();
        in_main_function = 0;
    }
    ;

struct_def:
    STRUCT ID LBRACE {
        // Inicia coleta de campos do struct
        current_struct_name = strdup($2);
        current_struct_fields = NULL;
    } struct_field_list RBRACE {
        // Define o struct na tabela de símbolos
        define_struct(current_struct_name, current_struct_fields);
        
        // Gera definição do struct em C
        emit_struct_definition(current_struct_name, current_struct_fields);
        
        // Limpa recursos
        free(current_struct_name);
        current_struct_name = NULL;
        current_struct_fields = NULL; // Não libera aqui, agora pertence à tabela de símbolos
    }
    ;

struct_field_list:
    struct_field
    | struct_field_list struct_field
    | struct_field_list NEWLINE
    | NEWLINE
    ;

struct_field:
    type ID SEMICOLON {
        // Adiciona campo ao struct atual
        add_struct_field(&current_struct_fields, $2, $1);
    }
    ;

block:
    LBRACE {
        char *scopeId = uniqueIdentifier();

        push_scope(scopeId);

        emit_code("{");
    } list_stmt RBRACE {
        pop_scope();
        emit_code("}");
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
            // Se for uma chamada de função, emite o código C
            if ($1 && $1->c_code) {
                emit_line("%s;", $1->c_code);
            }
        }
        free_expression_result($1);
    }
    | BREAK SEMICOLON {
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
        handle_while_body_end($1, $1 + 1);
        current_loop_exit_label = -1;
        exec_block = 1;
    }
    ;

while_start:
    WHILE LPAREN expression RPAREN {
        $$ = handle_while_condition($3);
        free_expression_result($3);
    }
    ;

if_stmt:
    if_start block {
        handle_if_end_part($1);
        exec_block = 1;
    }
    | if_start block ELSE {
        handle_if_else_part($1);
        exec_block = !last_condition_result;
    } block {
        handle_if_end_part(get_current_end_label());
        exec_block = 1;
    }
    | if_start block ELSE {
        handle_if_else_part($1);
        exec_block = !last_condition_result;
    } if_stmt {
        handle_if_end_part(get_current_end_label());
        exec_block = 1;
    }
    ;

if_start:
    IF LPAREN expression RPAREN {
        $$ = handle_if_condition($3);
        free_expression_result($3);
    }
    ;




for_stmt:
    FOR {
        char *scopeId = uniqueIdentifier();
        push_scope(scopeId);
        
        // Gera código C para início do for
        if (generate_code) {
            emit_code("for (");
            set_inline_mode(1); // Próximas emissões na mesma linha
        }
    } LPAREN for_init SEMICOLON {
        // Adiciona o primeiro semicolon do for
        if (generate_code) {
            emit_inline("; ");
        }
    } expression {
        // Adiciona a condição do for
        if (generate_code) {
            char* condition_code = $7->c_code ? $7->c_code : expression_to_c_code($7);
            emit_inline("%s; ", condition_code);
        }
        free_expression_result($7);
    } SEMICOLON assign_stmt RPAREN {
        // Fecha o cabeçalho do for
        if (generate_code) {
            emit_inline(") {");
            set_inline_mode(0);
            emit_line("");  // Adiciona a quebra de linha
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
        handle_for_init_declaration($1, $3, $5);
    }
    | assign_stmt {
        // Usa atribuição existente no for loop
    }
    ;

decl:
    /* empty */ { }
    | type COLON ID {
         handle_var_declaration($1, $3);
    }
    | type COLON ID ASSIGNMENT expression {
        handle_var_declaration_with_assignment($1, $3, $5);
        free_expression_result($5);
    }
    ;

type:
    TYPE                                { $$ = $1;}
    | ID                                { 
                                            // Isso pode ser um tipo struct
                                            StructDefinition* struct_def = find_struct_definition($1);
                                            if (struct_def) {
                                                $$ = strdup($1); //
                                            } else {
                                                semantic_error("Tipo não definido: '%s'", $1);
                                                YYABORT;
                                            }
                                        }
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
        add_function_parameter($1, $3);
        
        // Store parameter information for reference handling
        store_function_parameter(current_function_name, $3, $1);

        // INSERIR AQUI a mesma lógica de inserção na tabela de símbolos
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
    | type '&' COLON ID {
        char* ref_type = malloc(strlen($1) + 2);
        sprintf(ref_type, "%s&", $1);
        add_function_parameter(ref_type, $4);
        
        // Store parameter information for reference handling
        store_function_parameter(current_function_name, $4, ref_type);

        if (find_variable_in_current_scope($4) != NULL) {
            semantic_error("Parâmetro '%s' já declarado na função.", $4);
        } else {
            char *fullKey = malloc(strlen(currentScope) + strlen($4) + 2);
            sprintf(fullKey, "%s#%s", currentScope, $4);
            Data data;
            data.type = strdup(ref_type);
            insert_node(&symbolTable, fullKey, data);
            free(fullKey);
        }

        free(ref_type);
    }
;


return_stmt:
    RETURN expression { 
        handle_return_statement($2);
        free_expression_result($2);
    }
    ;

print_stmt: 
    PRINT LPAREN print_arg_list RPAREN {
        handle_print_statement();
    }
    ;// gustavo


read_stmt:
    READ LPAREN lvalue RPAREN {
        handle_read_statement($3);
        free_lvalue_result($3);
    }
    ;

print_arg_list:
    print_arg
    | print_arg_list COMMA print_arg
    ;

print_arg:
    expression {
        handle_print_expression($1);
        free_expression_result($1);
    }
    ;

assign_stmt:
    lvalue ASSIGNMENT expression {
        handle_assignment($1, $3);
        free_lvalue_result($1);
        free_expression_result($3);
    }
    | lvalue INCREMENT {
        handle_increment($1);
        free_lvalue_result($1);
    }
    | lvalue DECREMENT {
        handle_decrement($1);
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
            // Acesso a array multidimensional: arr[i][j]
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
            
            // Calcula o novo tipo do elemento removendo uma dimensão
            char* newElementType = get_array_element_type($1->elementType);
            if (!newElementType) {
                semantic_error("Erro: tentativa de indexar um tipo que não é array.");
                YYABORT;
            }
            
            // Cria novo lvalue com mais uma dimensão
            $$ = create_lvalue_multidim_access($1->varName, newElementType, 
                                               $1->dimensionCount + 1, newIndices);
            
            // Limpa memória temporária
            for (int i = 0; i <= $1->dimensionCount; i++) {
                free(newIndices[i]);
            }
            free(newIndices);
            free(newElementType);
            free_lvalue_result($1);
            free_expression_result($3);
            
        } else if ($1->type == LVALUE_VAR) {
            // Acesso a array unidimensional: arr[i]
            Data* arrayNode = find_variable_in_scopes($1->varName);
            if (!arrayNode) {
                semantic_error("Variável '%s' não declarada.", $1->varName);
                YYABORT;
            }
            
            if (!strstr(arrayNode->type, "[]")) {
                semantic_error("Tentativa de indexar uma variável que não é um array: '%s'.", $1->varName);
                YYABORT;
            }
            
            // Verifica se o índice é um tipo válido (int)
            if (strcmp($3->type, "int") != 0) {
                semantic_error("Índice de array deve ser do tipo int, mas foi '%s'.", $3->type);
                YYABORT;
            }
            
            // Determina o tipo do elemento
            char* elementType = get_array_element_type(arrayNode->type);
            
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
    | lvalue DOT ID {
        // Struct field access: struct_var.field_name
        if ($1->type != LVALUE_VAR) {
            semantic_error("Acesso a campo só é permitido em variáveis simples, não em arrays ou outros tipos.");
            YYABORT;
        }
        
        // Find the variable to get its type
        Data* structNode = find_variable_in_scopes($1->varName);
        if (!structNode) {
            semantic_error("Variável '%s' não declarada.", $1->varName);
            YYABORT;
        }
        
        // Check if the variable is a struct type
        StructDefinition* struct_def = find_struct_definition(structNode->type);
        if (!struct_def) {
            semantic_error("Variável '%s' não é um struct.", $1->varName);
            YYABORT;
        }
        
        // Find the field in the struct
        StructField* field = find_struct_field(structNode->type, $3);
        if (!field) {
            semantic_error("Campo '%s' não existe no struct '%s'.", $3, structNode->type);
            YYABORT;
        }
        
        // Create struct field lvalue
        $$ = create_lvalue_struct_field($1->varName, $3, structNode->type, field->type);
        
        free_lvalue_result($1);
    }
    ;

expression:
    BOOL {
        $$ = create_bool_expression($1);
    }
    | INT {
        $$ = create_int_expression($1);
    }
    | FLOAT {
        $$ = create_float_expression($1);
    }
    | STRING {
        $$ = create_string_expression($1);
    }
    | lvalue {
        $$ = create_lvalue_expression($1);
        if (!$$) YYABORT;
        free_lvalue_result($1);
    }
    | LPAREN expression RPAREN {
          $$ = $2;
      }
    | expression ADDITION expression {
        $$ = handle_addition($1, $3);
        free_expression_result($1);
        free_expression_result($3);
    }
    | expression SUBTRACTION expression {
        $$ = handle_subtraction($1, $3);
        free_expression_result($1);
        free_expression_result($3);
    }
    | expression MULTIPLICATION expression {
        $$ = handle_multiplication($1, $3);
        free_expression_result($1);
        free_expression_result($3);
    }
    | expression DIVISION expression {
        $$ = handle_division($1, $3);
        if (!$$) YYABORT;
        free_expression_result($1);
        free_expression_result($3);
    }
    | expression EXPONENTIATION expression {
        $$ = handle_exponentiation($1, $3);
        free_expression_result($1);
        free_expression_result($3);
    }
    | expression SMALLER expression {
        $$ = handle_less_than($1, $3);
        if (!$$) YYABORT;
        free_expression_result($1);
        free_expression_result($3);
    }
    | expression BIGGER expression {
        $$ = handle_greater_than($1, $3);
        if (!$$) YYABORT;
        free_expression_result($1);
        free_expression_result($3);
    }
    | expression SMALLEREQUALS expression {
        $$ = handle_less_equal($1, $3);
        if (!$$) YYABORT;
        free_expression_result($1);
        free_expression_result($3);
    }
    | expression BIGGEREQUALS expression {
        $$ = handle_greater_equal($1, $3);
        if (!$$) YYABORT;
        free_expression_result($1);
        free_expression_result($3);
    }
    | expression EQUALS expression {
        $$ = handle_equals($1, $3);
        if (!$$) YYABORT;
        free_expression_result($1);
        free_expression_result($3);
    }
    | expression AND expression {
        $$ = handle_logical_and($1, $3);
        if (!$$) YYABORT;
        free_expression_result($1);
        free_expression_result($3);
    }
    | expression OR expression {
        $$ = handle_logical_or($1, $3);
        if (!$$) YYABORT;
        free_expression_result($1);
        free_expression_result($3);
    }
    | expression MODULO expression {
        $$ = handle_modulo($1, $3);
        free_expression_result($1);
        free_expression_result($3);
    }
    | SUBTRACTION expression %prec UMINUS {
        $$ = handle_unary_minus($2);
        if (!$$) YYABORT;
        free_expression_result($2);
    }
    | ID LPAREN arg_list_opt RPAREN {
        $$ = handle_function_call_with_args($1, get_function_arguments());
        clear_function_arguments();
    }
    | LEN LPAREN expression RPAREN {
        $$ = handle_len_expression($3);
        free_expression_result($3);
    }
    | LBRACKET array_values RBRACKET {
        $$ = handle_array_literal_with_values($2);
        free($2);
    }
    ;

arg_list_opt:
                                                   { 
        clear_function_arguments(); // Clear any previous arguments
    }
    | arg_list                                     { /* lista de argumentos já coletada */ }
    ;

arg_list:
    expression                                     { 
        clear_function_arguments(); // Clear for first argument
        if ($1->c_code) {
            add_function_argument($1->c_code);
        } else {
            // Convert expression value to string
            char temp[100];
            if (strcmp($1->type, "int") == 0) {
                sprintf(temp, "%d", $1->intVal);
            } else if (strcmp($1->type, "float") == 0) {
                sprintf(temp, "%f", $1->doubleVal);
            } else if (strcmp($1->type, "bool") == 0) {
                sprintf(temp, "%d", $1->intVal);
            } else if (strcmp($1->type, "string") == 0 && $1->strVal) {
                sprintf(temp, "%s", $1->strVal);
            } else {
                sprintf(temp, "0");
            }
            add_function_argument(temp);
        }
        free_expression_result($1);
    }
    | arg_list COMMA expression                    { 
        if ($3->c_code) {
            add_function_argument($3->c_code);
        } else {
            // Convert expression value to string
            char temp[100];
            if (strcmp($3->type, "int") == 0) {
                sprintf(temp, "%d", $3->intVal);
            } else if (strcmp($3->type, "float") == 0) {
                sprintf(temp, "%f", $3->doubleVal);
            } else if (strcmp($3->type, "bool") == 0) {
                sprintf(temp, "%d", $3->intVal);
            } else if (strcmp($3->type, "string") == 0 && $3->strVal) {
                sprintf(temp, "%s", $3->strVal);
            } else {
                sprintf(temp, "0");
            }
            add_function_argument(temp);
        }
        free_expression_result($3);
    }
    ;

array_values:
    expression                                     { 
        // Início da lista de valores do array
        if ($1->c_code) {
            $$ = strdup($1->c_code);
        } else {
            $$ = strdup("0"); // valor padrão
        }
        free_expression_result($1);
    }
    | array_values COMMA expression                { 
        // Adiciona novo valor à lista
        char* new_values = malloc(strlen($1) + strlen($3->c_code ? $3->c_code : "0") + 3);
        sprintf(new_values, "%s, %s", $1, $3->c_code ? $3->c_code : "0");
        free($1);
        $$ = new_values;
        free_expression_result($3);
    }
    ;

%%

void yyerror(const char* s) {
    extern int yylineno;
    fprintf(stderr, "Erro de Sintaxe: erro de sintaxe na linha %d\n", yylineno);
    syntax_errors++;
}

// Helper functions for parameter collection
void add_function_parameter(const char* type, const char* name) {
    FunctionParam* param = malloc(sizeof(FunctionParam));
    param->type = strdup(type);
    param->name = strdup(name);
    param->next = current_function_params;
    current_function_params = param;
}

void clear_function_parameters() {
    while (current_function_params) {
        FunctionParam* temp = current_function_params;
        current_function_params = current_function_params->next;
        free(temp->type);
        free(temp->name);
        free(temp);
    }
    // Don't clear function name and return type here - they should persist during function parsing
}

void clear_function_context() {
    if (current_function_name) {
        free(current_function_name);
        current_function_name = NULL;
    }
    if (current_function_return_type) {
        free(current_function_return_type);
        current_function_return_type = NULL;
    }
}

void emit_function_signature() {
    if (!generate_code) return;
    if (!current_function_name || !current_function_return_type) return;
    
    if (strcmp(current_function_name, "main") == 0) {
        emit_line("int main() {");
        increase_indent();
        return;
    }
    
    // Convert return type to C
    char* c_return_type = convert_penelope_type_to_c(current_function_return_type);
    if (!c_return_type) {
        c_return_type = strdup("int"); // fallback
    }
    
    // Build parameter list
    char param_list[1000] = "";
    
    if (current_function_params) {
        // Collect parameters in reverse order (since we added them backwards)
        FunctionParam* params[100];
        int param_count = 0;
        
        FunctionParam* curr = current_function_params;
        while (curr && param_count < 100) {
            params[param_count++] = curr;
            curr = curr->next;
        }
        
        // Build parameter string in correct order
        for (int i = param_count - 1; i >= 0; i--) {
            char* c_param_type = convert_penelope_type_to_c(params[i]->type);
            if (c_param_type) {
                if (strlen(param_list) > 0) {
                    strcat(param_list, ", ");
                }
                strcat(param_list, c_param_type);
                strcat(param_list, " ");
                strcat(param_list, params[i]->name);
            }
        }
    }
    
    emit_line("%s %s(%s) {", c_return_type, current_function_name, param_list);
    increase_indent();
}

void emit_function_signature_with_info(const char* return_type, const char* function_name) {
    if (!generate_code) return;
    if (!function_name || !return_type) return;
    
    if (strcmp(function_name, "main") == 0) {
        emit_line("int main() {");
        increase_indent();
        return;
    }
    
    // Convert return type to C
    char* c_return_type = convert_penelope_type_to_c(return_type);
    if (!c_return_type) {
        c_return_type = strdup("int"); // fallback
    }
    
    // Build parameter list
    char param_list[1000] = "";
    
    if (current_function_params) {
        // Collect parameters in reverse order (since we added them backwards)
        FunctionParam* params[100];
        int param_count = 0;
        
        FunctionParam* curr = current_function_params;
        while (curr && param_count < 100) {
            params[param_count++] = curr;
            curr = curr->next;
        }
        
        // Build parameter string in correct order
        for (int i = param_count - 1; i >= 0; i--) {
            char* c_param_type = convert_penelope_type_to_c(params[i]->type);
            if (c_param_type) {
                if (strlen(param_list) > 0) {
                    strcat(param_list, ", ");
                }
                strcat(param_list, c_param_type);
                strcat(param_list, " ");
                strcat(param_list, params[i]->name);
            }
        }
    }
    
    emit_line("%s %s(%s) {", c_return_type, function_name, param_list);
    increase_indent();
}

// Argument collection functions
void add_function_argument(const char* arg_code) {
    FunctionArg* arg = malloc(sizeof(FunctionArg));
    arg->code = strdup(arg_code);
    arg->next = current_function_args;
    current_function_args = arg;
}

void clear_function_arguments() {
    while (current_function_args) {
        FunctionArg* temp = current_function_args;
        current_function_args = current_function_args->next;
        free(temp->code);
        free(temp);
    }
}

char* get_function_arguments() {
    if (!current_function_args) return strdup("");
    
    char* result = malloc(1000);
    result[0] = '\0';
    
    // Build argument list in reverse order (since we added them backwards)
    FunctionArg* args[100]; // temporary array to reverse
    int arg_count = 0;
    
    // Collect arguments in array
    FunctionArg* curr = current_function_args;
    while (curr && arg_count < 100) {
        args[arg_count++] = curr;
        curr = curr->next;
    }
    
    // Build argument string in correct order
    for (int i = arg_count - 1; i >= 0; i--) {
        if (strlen(result) > 0) {
            strcat(result, ", ");
        }
        strcat(result, args[i]->code);
    }
    
    return result;
}