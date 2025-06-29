%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./utils/hashMap/hashMap.h"
#include "./utils/uniqueIdentifier/uniqueIdentifier.h"

#define MAX_SCOPE_DEPTH 100
char* scopeStack[MAX_SCOPE_DEPTH];
int scopeTop = -1;

HashMap symbolTable = { NULL };
char *currentScope = NULL;

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
}

%token <str> ID TYPE NUMBER STRING

%token FUN WHILE FOR IF ELSE LEN RETURN
%token LBRACKET RBRACKET COMMA LPAREN RPAREN COLON SEMICOLON NEWLINE
%token ASSIGNMENT EQUALS SMALLEREQUALS BIGGEREQUALS SMALLER BIGGER
%token INCREMENT DECREMENT EXPONENTIATION MULTIPLICATION DIVISION ADDITION SUBTRACTION
%token LBRACE RBRACE

%define parse.trace

%type <str> expression lvalue type list_expression

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
    FOR LPAREN for_init SEMICOLON expression SEMICOLON assign_stmt RPAREN block
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
          free(fullKey);
      }
    | type COLON ID ASSIGNMENT expression {
          char *fullKey = malloc(strlen(currentScope) + strlen($3) + 2);
          sprintf(fullKey, "%s#%s", currentScope, $3);
          insert_node(&symbolTable, fullKey, $1);
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

assign_stmt:
    lvalue ASSIGNMENT expression
    | lvalue INCREMENT
    | lvalue DECREMENT
    ;

lvalue:
    ID
    | lvalue LBRACKET expression RBRACKET
    ;

expression:
    NUMBER                                         { $$ = $1; }
    | lvalue                                       { $$ = $1; }
    | STRING                                       { $$ = $1; }
    | LPAREN expression RPAREN                     { $$ = $2; }
    | expression ADDITION expression               { $$ = strdup("expr"); }
    | expression SUBTRACTION expression            { $$ = strdup("expr"); }
    | expression MULTIPLICATION expression         { $$ = strdup("expr"); }
    | expression DIVISION expression               { $$ = strdup("expr"); }
    | expression EXPONENTIATION expression         { $$ = strdup("expr"); }
    | expression SMALLER expression                { $$ = strdup("expr"); }
    | expression BIGGER expression                 { $$ = strdup("expr"); }
    | expression SMALLEREQUALS expression          { $$ = strdup("expr"); }
    | expression BIGGEREQUALS expression           { $$ = strdup("expr"); }
    | expression EQUALS expression                 { $$ = strdup("expr"); }
    | SUBTRACTION expression %prec UMINUS          { $$ = strdup("expr"); }
    | ID LPAREN arg_list_opt RPAREN                { $$ = strdup("expr"); }
    | LEN LPAREN expression RPAREN                 { $$ = strdup("expr"); }
    | LBRACKET list_expression RBRACKET            { $$ = strdup("array_literal"); }
    ;

list_expression:
    expression
    | list_expression COMMA expression
    ;

arg_list_opt:
    | arg_list
    ;

arg_list:
    expression
    | arg_list COMMA expression
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

    if (yyparse() == 0) {
        printf("Análise concluída com sucesso. A sintaxe está correta!\n");
    } else {
        printf("Falha na análise. Foi encontrado um erro de sintaxe.\n");
    }

    print_map(&symbolTable);
    free_map(&symbolTable);

    return 0;
}
