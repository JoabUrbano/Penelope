%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yydebug = 0;  // Define the variable here

extern int yylex();
extern int yyparse();
extern FILE* yyin;
void yyerror(const char* s);

extern int indent_stack[];
extern int indent_sp;

%}

%union {
    char *str;
}

%token <str> ID TYPE NUMBER STRING

%token FUN WHILE FOR IF ELSE LEN RETURN
%token LBRACKET RBRACKET COMMA LPAREN RPAREN COLON SEMICOLON NEWLINE
%token ASSIGNMENT EQUALS SMALLEREQUALS BIGGEREQUALS SMALLER BIGGER
%token INCREMENT DECREMENT EXPONENTIATION MULTIPLICATION DIVISION ADDITION SUBTRACTION

%token INDENT DEDENT COLON_NEWLINE

%define parse.trace

%type <str> expression lvalue type

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
    | list_decl_fun decl_or_fun
    ;

decl_or_fun:
    fun
    | decl SEMICOLON
    | NEWLINE
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
    simple_stmt
    | compound_stmt
    | NEWLINE
    ;

simple_stmt:
    decl SEMICOLON
    | decl NEWLINE
    | assign_stmt SEMICOLON
    | assign_stmt NEWLINE
    | return_stmt SEMICOLON
    | return_stmt NEWLINE
    | expression SEMICOLON
    | expression NEWLINE
    ;

compound_stmt:
    if_stmt
    | for_stmt
    | while_stmt
    ;

while_stmt:
    WHILE LPAREN expression RPAREN COLON_NEWLINE block
    ;

decl:
    type COLON ID
    | type COLON ID ASSIGNMENT expression
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

if_stmt:
    IF LPAREN expression RPAREN COLON_NEWLINE block %prec LOWER_THAN_ELSE
    | IF LPAREN expression RPAREN COLON_NEWLINE block ELSE COLON_NEWLINE block
    ;

for_stmt:
    FOR LPAREN for_init SEMICOLON expression SEMICOLON assign_stmt RPAREN COLON_NEWLINE block
    ;

for_init:
    decl
    | assign_stmt
    ;

return_stmt:
    RETURN expression
    ;

assign_stmt:
    lvalue ASSIGNMENT expression
    | ID INCREMENT
    | ID DECREMENT
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

    indent_stack[0] = 0;
    indent_sp = 1;
    
    yydebug = 1;  // Enable parser debugging
    
    if (yyparse() == 0) {
        printf("Análise concluída com sucesso. A sintaxe está correta!\n");
    } else {
        printf("Falha na análise. Foi encontrado um erro de sintaxe.\n");
    }

    return 0;
}