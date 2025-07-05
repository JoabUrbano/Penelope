%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "./utils/hashMap/hashMap.h"
#include "./utils/uniqueIdentifier/uniqueIdentifier.h"

#define MAX_SCOPE_DEPTH 100
char* scopeStack[MAX_SCOPE_DEPTH];
int scopeTop = -1;

HashMap symbolTable = { NULL };
HashMap valueTable = { NULL };  // Store actual values
char *currentScope = NULL;
int semantic_errors = 0; 

void semantic_error(const char* format, const char* name) {
    extern int yylineno;
    fprintf(stderr, "Erro Semântico na linha %d: ", yylineno);
    fprintf(stderr, format, name);
    fprintf(stderr, "\n");
    semantic_errors++;
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


double evaluate_number(char *str) {
    return atof(str);
}

void print_value(double value) {
    printf("%.6g ", value);  // Print with space, no newline, up to 6 significant digits
}

void print_string(char *str) {
    // Remove quotes from string literals
    if (str && str[0] == '"' && str[strlen(str)-1] == '"') {
        str[strlen(str)-1] = '\0';  // Remove ending quote
        printf("%s ", str + 1);   // Skip beginning quote, add space
    } else if (str) {
        printf("%s ", str);
    }
}

void print_newline() {
    printf("\n");
}

double power_operation(double base, double exponent) {
    return pow(base, exponent);
}

void store_variable_value(char *name, double value) {
    char *fullKey = malloc(strlen(currentScope) + strlen(name) + 2);
    sprintf(fullKey, "%s#%s", currentScope, name);
    
    char *valueStr = malloc(32);
    sprintf(valueStr, "%.10g", value);
    insert_node(&valueTable, fullKey, valueStr);
    
    free(fullKey);
    free(valueStr);
}

double get_variable_value(char *name) {
    for (int i = scopeTop; i >= 0; i--) {
        char *scope = scopeStack[i];
        char *fullKey = malloc(strlen(scope) + strlen(name) + 2);
        sprintf(fullKey, "%s#%s", scope, name);

        Node* node = find_node(&valueTable, fullKey);
        free(fullKey);

        if (node) {
            return atof(node->value);
        }
    }
    return 0.0;
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

%union {
    char *str;
    double num;
}

%token <str> ID TYPE STRING
%token <num> NUMBER

%token FUN WHILE FOR IF ELSE LEN PRINT RETURN
%token LBRACKET RBRACKET COMMA LPAREN RPAREN COLON SEMICOLON NEWLINE
%token ASSIGNMENT EQUALS SMALLEREQUALS BIGGEREQUALS SMALLER BIGGER
%token INCREMENT DECREMENT EXPONENTIATION MULTIPLICATION DIVISION ADDITION SUBTRACTION
%token LBRACE RBRACE

%define parse.trace

%type <str> lvalue type list_expression
%type <num> expression

%right ASSIGNMENT
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
    FUN type ID LPAREN list_param_opt RPAREN block
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
    | expression SEMICOLON
    ;

compound_stmt:
    if_stmt
    | for_stmt
    | while_stmt
    ;

while_stmt:
    WHILE LPAREN expression RPAREN block
    ;

if_stmt:
    IF LPAREN expression RPAREN block %prec LOWER_THAN_ELSE
    | IF LPAREN expression RPAREN block ELSE block
    ;

for_stmt:
    FOR {
        char *scopeId = uniqueIdentifier();

        push_scope(scopeId);
    } LPAREN for_init SEMICOLON expression SEMICOLON assign_stmt RPAREN LBRACE list_stmt RBRACE {
        pop_scope();
    }
    ;

for_init:
    decl
    | assign_stmt
    ;

decl:
      type COLON ID {
          char *fullKey = malloc(strlen(currentScope) + strlen($3) + 2);
          sprintf(fullKey, "%s#%s", currentScope, $3);
          insert_node(&symbolTable, fullKey, $1);
          store_variable_value($3, 0.0); // Initialize with 0
          free(fullKey);
      }
    | type COLON ID ASSIGNMENT expression {
          char *fullKey = malloc(strlen(currentScope) + strlen($3) + 2);
          sprintf(fullKey, "%s#%s", currentScope, $3);
          insert_node(&symbolTable, fullKey, $1);
          store_variable_value($3, $5); // Store the computed value
          free(fullKey);
      }
    ;

type:
    TYPE                                { $$ = $1; }
    | TYPE LBRACKET RBRACKET            { 
                                            char *array_type = malloc(strlen($1) + 3);
                                            sprintf(array_type, "%s[]", $1);
                                            $$ = array_type;
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
    type COLON ID
    ;

return_stmt:
    RETURN expression
    ;

print_stmt:
    PRINT LPAREN print_arg_list RPAREN {
        print_newline();
    }
    ;

print_arg_list:
    print_arg
    | print_arg_list COMMA print_arg
    ;

print_arg:
    expression {
        print_value($1);
    }
    | STRING {
        print_string($1);
    }
    ;

assign_stmt:
    lvalue ASSIGNMENT expression {
        // Verifica se a variável do lvalue foi declarada antes de atribuir
        if ($1 && strcmp($1, "array_access") != 0) {
            if (find_variable_in_scopes($1) == NULL) {
                semantic_error("Variável '%s' não declarada.", $1);
            } else {
                store_variable_value($1, $3);
            }
        }
    }
    | lvalue INCREMENT {
        if ($1 && strcmp($1, "array_access") != 0) {
            if (find_variable_in_scopes($1) == NULL) {
                semantic_error("Variável '%s' não declarada.", $1);
            } else {
                double current = get_variable_value($1);
                store_variable_value($1, current + 1.0);
            }
        }
    }
    | lvalue DECREMENT {
        if ($1 && strcmp($1, "array_access") != 0) {
             if (find_variable_in_scopes($1) == NULL) {
                semantic_error("Variável '%s' não declarada.", $1);
            } else {
                double current = get_variable_value($1);
                store_variable_value($1, current - 1.0);
            }
        }
    }
    ;

lvalue:
    ID { 
        $$ = $1; 
        // A verificação é feita no contexto (assign_stmt ou expression)
    }
    | lvalue LBRACKET expression RBRACKET { 
        $$ = strdup("array_access"); 
    }
    ;

expression:
    NUMBER                                         { $$ = $1; }
    | STRING                                       { $$ = 0.0; /* String literals not implemented in expressions */ }
    | lvalue {
        if ($1 && strcmp($1, "array_access") != 0) {
            // VERIFICAÇÃO SEMÂNTICA AQUI
            if (find_variable_in_scopes($1) == NULL) {
                semantic_error("Variável '%s' não declarada.", $1);
                $$ = 0.0; // Retorna um valor padrão para continuar a análise
            } else {
                $$ = get_variable_value($1);
            }
        } else {
            $$ = 0.0; // Valor padrão para acesso a array (ainda não implementado)
        }
    }
    | LPAREN expression RPAREN                     { $$ = $2; }
    | expression ADDITION expression               { $$ = $1 + $3; }
    | expression SUBTRACTION expression            { $$ = $1 - $3; }
    | expression MULTIPLICATION expression         { $$ = $1 * $3; }
    | expression DIVISION expression               { $$ = $1 / $3; }
    | expression EXPONENTIATION expression         { $$ = power_operation($1, $3); }
    | expression SMALLER expression                { $$ = ($1 < $3) ? 1.0 : 0.0; }
    | expression BIGGER expression                 { $$ = ($1 > $3) ? 1.0 : 0.0; }
    | expression SMALLEREQUALS expression          { $$ = ($1 <= $3) ? 1.0 : 0.0; }
    | expression BIGGEREQUALS expression           { $$ = ($1 >= $3) ? 1.0 : 0.0; }
    | expression EQUALS expression                 { $$ = ($1 == $3) ? 1.0 : 0.0; }
    | SUBTRACTION expression %prec UMINUS          { $$ = -$2; }
    | ID LPAREN arg_list_opt RPAREN                { $$ = 0.0; /* Function calls not implemented yet */ }
    | LEN LPAREN expression RPAREN                 { $$ = 0.0; /* len() not implemented yet */ }
    | LBRACKET list_expression RBRACKET            { $$ = 0.0; /* Array literals not implemented yet */ }
    ;

list_expression:
    expression                                     { $$ = strdup("array_element"); }
    | list_expression COMMA expression             { $$ = strdup("array_list"); }
    ;

arg_list_opt:
                                                   { /* empty */ }
    | arg_list                                     { /* argument list */ }
    ;

arg_list:
    expression                                     { /* single argument */ }
    | arg_list COMMA expression                    { /* multiple arguments */ }
    ;

%%

void yyerror(const char* s) {
    extern int yylineno;
    fprintf(stderr, "Erro de Sintaxe: %s na linha %d\n", s, yylineno);
}

int main(int argc, char **argv) {
    if (argc > 1) {
        FILE *file = fopen(argv[1], "r");
        if (!file) {
            perror(argv[1]);
            return 1;
        }
        yyin = file;
    } else {
        yyin = stdin;
    }

    push_scope(strdup("global"));

    if (yyparse() == 0 && semantic_errors == 0) {
        printf("Análise concluída com sucesso. A sintaxe e a semântica estão corretas!\n");
    } else {
        printf("Falha na análise. Foram encontrados %d erros semânticos e/ou erros de sintaxe.\n", semantic_errors);
    }

    print_map(&symbolTable);
    
    // Libera o escopo global
    pop_scope(); 

    free_map(&symbolTable);
    free_map(&valueTable);

    return (semantic_errors > 0);
}
