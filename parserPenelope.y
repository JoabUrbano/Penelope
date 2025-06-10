%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yylex();
extern int yyparse();
extern FILE* yyin;
void yyerror(const char* s);

extern int indent_stack[];
extern int indent_sp;

%}

/* União para armazenar diferentes tipos de valores para os tokens. */
%union {
    char *str;
}

/* Declara os tokens que possuem valores (vindos do lexer). */
%token <str> ID TYPE NUMBER STRING

/* Declara todos os outros tokens (palavras-chave, operadores, pontuação). */
%token FUN WHILE FOR IF ELSE LEN RETURN
%token LBRACKET RBRACKET COMMA LPAREN RPAREN COLON SEMICOLON
%token ASSIGNMENT EQUALS SMALLEREQUALS BIGGEREQUALS SMALLER BIGGER
%token INCREMENT DECREMENT EXPONENTIATION MULTIPLICATION DIVISION ADDITION SUBTRACTION

/* Tokens que lidam com a indentação. */
%token INDENT DEDENT COLON_NEWLINE

/* Precedência e Associatividade dos Operadores (da menor para a maior). */
%right ASSIGNMENT
%left EQUALS
%left SMALLEREQUALS BIGGEREQUALS SMALLER BIGGER
%left ADDITION SUBTRACTION
%left MULTIPLICATION DIVISION
%right EXPONENTIATION
%nonassoc UMINUS 

/* Define a regra inicial da gramática. */
%start program

%%

/* ---------- REGRAS DA GRAMÁTICA ---------- */

program:
    list_decl_fun
    ;

list_decl_fun:
    | list_decl_fun decl_or_fun
    ;

decl_or_fun:
    fun
    | decl SEMICOLON
    ;

fun:
    FUN type ID LPAREN list_param_opt RPAREN COLON_NEWLINE block
    ;

block:
    INDENT list_stmt DEDENT
    ;

list_stmt:
    stmt
    | list_stmt stmt
    ;

stmt:
    simple_stmt SEMICOLON
    | compound_stmt
    ;

simple_stmt:
    decl
    | assign
    | return_stmt
    ;

compound_stmt:
    if_stmt
    | for_stmt
    ;

decl:
    type COLON ID
    | type COLON ID ASSIGNMENT expression
    ;

type:
    TYPE 
    | ID
    | TYPE LBRACKET RBRACKET
    | ID LBRACKET RBRACKET
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

if_stmt:
    IF LPAREN expression RPAREN COLON_NEWLINE block
    | IF LPAREN expression RPAREN COLON_NEWLINE block ELSE COLON_NEWLINE block
    ;

for_stmt:
    FOR LPAREN for_init SEMICOLON expression SEMICOLON assign RPAREN COLON_NEWLINE block
    ;

for_init:
    decl
    | assign
    ;

return_stmt:
    RETURN expression
    ;

assign:
    ID ASSIGNMENT expression
    | ID INCREMENT
    | ID DECREMENT
    ;

expression:
    NUMBER
    | ID
    | STRING
    | LPAREN expression RPAREN                      { $$ = $2; }
    | expression ADDITION expression
    | expression SUBTRACTION expression
    | expression MULTIPLICATION expression
    | expression DIVISION expression
    | expression EXPONENTIATION expression
    | expression SMALLER expression
    | expression BIGGER expression
    | expression SMALLEREQUALS expression
    | expression BIGGEREQUALS expression
    | expression EQUALS expression
    | SUBTRACTION expression %prec UMINUS          /* Menos unário. */
    | ID LPAREN arg_list_opt RPAREN                /* Chamada de função. */
    | expression LBRACKET expression RBRACKET      /* Acesso a array. */
    | assign                                       /* Atribuição */
    | LEN LPAREN expression RPAREN
    ;

arg_list_opt:
    | arg_list
    ;

arg_list:
    expression
    | arg_list COMMA expression
    ;

%%

/* ---------- SEÇÃO DE CÓDIGO C ---------- */

void yyerror(const char* s) {
    fprintf(stderr, "Erro de Sintaxe: %s\n", s);
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

    indent_stack[0] = 0;
    indent_sp = 1;

    // Inicia a análise sintática (parsing).
    if (yyparse() == 0) {
        printf("Análise concluída com sucesso. A sintaxe está correta!\n");
    } else {
        printf("Falha na análise. Foi encontrado um erro de sintaxe.\n");
    }

    return 0;
}