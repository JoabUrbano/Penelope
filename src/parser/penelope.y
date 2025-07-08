%{
#include "../src/parser/grammarActions/grammarActions.h"
#include "../src/parser/codeGenerator/codeGenerator.h"
#include "../src/parser/semantics/semantics.h"
#include "../src/parser/symbolTable/symbolTable.h"
#include "../src/parser/expressionUtils/expressionUtils.h"
#include "../src/utils/uniqueIdentifier/uniqueIdentifier.h"
#include "../src/structs/lvalue/lvalueResult.h"

// Forward declarations for Bison
extern int yylineno;
extern int yylex();
extern void yyerror(const char* s);

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

%token <num> INT FLOAT

%token FUN WHILE FOR IF ELSE LEN PRINT READ RETURN
%token LBRACKET RBRACKET COMMA LPAREN RPAREN COLON SEMICOLON NEWLINE
%token ASSIGNMENT EQUALS SMALLEREQUALS BIGGEREQUALS SMALLER BIGGER
%token INCREMENT DECREMENT EXPONENTIATION MULTIPLICATION DIVISION ADDITION SUBTRACTION
%token LBRACE RBRACE

%define parse.trace

%type <lvalueResult> lvalue
%type <str> type array_values
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
        // Armazena informações da função na tabela de símbolos
        store_function($3, $2);
        
        // Verifica se é a função main
        if (strcmp($3, "main") == 0) {
            in_main_function = 1;
            enable_code_generation();
            // Emite o início da função main
            if (generate_code) {
                emit_line("int main() {");
                increase_indent();
            }
        } else {
            in_main_function = 0;
            disable_code_generation(); // Desabilita geração para funções que não são main
        }
        
        // Cria o escopo da função antes de processar parâmetros
        char *scopeId = uniqueIdentifier();
        push_scope(scopeId);
    } list_param_opt RPAREN LBRACE list_stmt RBRACE {
        // Remove o escopo da função
        pop_scope();
        
        // Fecha a função main se estávamos nela
        if (in_main_function) {
            if (generate_code) {
                emit_line("return 0;");
                decrease_indent();
                emit_line("}");
            }
        } else {
            // Reabilita geração de código após processar função não-main
            enable_code_generation();
        }
        
        in_main_function = 0;
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
    | if_start block ELSE if_stmt {
        handle_if_else_part($1);
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
            emit_line("");  // Add the newline
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
            // Single-dimensional array access: arr[i]
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
    | SUBTRACTION expression %prec UMINUS {
        $$ = handle_unary_minus($2);
        if (!$$) YYABORT;
        free_expression_result($2);
    }
    | ID LPAREN arg_list_opt RPAREN {
        $$ = handle_function_call($1);
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
                                                   { /* vazio */ }
    | arg_list                                     { /* lista de argumentos */ }
    ;

arg_list:
    expression                                     { /* argumento único */ }
    | arg_list COMMA expression                    { /* múltiplos argumentos */ }
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
