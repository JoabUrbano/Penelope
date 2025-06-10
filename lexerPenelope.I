%{
#include <stdio.h>
#include <string.h>
#include "parser.tab.h"

#define MAX_INDENT_STACK 100
#define INDENT_WIDTH 4

int token_queue[MAX_INDENT_STACK];
int queue_head = 0;
int queue_tail = 0;

int indent_stack[MAX_INDENT_STACK];
int indent_sp;

int at_line_start = 1;

void push_token(int token) {
    if (queue_tail >= MAX_INDENT_STACK) {
        fprintf(stderr, "Erro Léxico: Estouro da fila de tokens. Muitos DEDENTs?\n");
        exit(1);
    }
    token_queue[queue_tail++] = token;
}

void handle_indent(int new_indent_spaces) {
    if (new_indent_spaces % INDENT_WIDTH != 0 && new_indent_spaces != 0) {
        fprintf(stderr, "Erro Léxico: Indentação inválida de %d espaços. Deve ser um múltiplo de %d.\n", new_indent_spaces, INDENT_WIDTH);
        exit(1);
    }
    int new_indent_level = new_indent_spaces / INDENT_WIDTH;

    if (new_indent_level > indent_stack[indent_sp - 1]) {
        if (indent_sp >= MAX_INDENT_STACK) {
            fprintf(stderr, "Erro Léxico: Estouro da pilha de indentação\n");
            exit(1);
        }
        push_token(INDENT);
        indent_stack[indent_sp++] = new_indent_level;
    } else {
        while (new_indent_level < indent_stack[indent_sp - 1]) {
            indent_sp--;
            push_token(DEDENT);
        }
        if (new_indent_level != indent_stack[indent_sp - 1]) {
            fprintf(stderr, "Erro Léxico: Nível de indentação inconsistente.\n");
            exit(1);
        }
    }
}

int process_queue() {
    if (queue_head < queue_tail) {
        return token_queue[queue_head++];
    }
    queue_head = 0;
    queue_tail = 0;
    return 0;
}

%}

%option noyywrap
%option yylineno

%%

%{
    int token;
    if ((token = process_queue())) {
        return token;
    }
%}

^[ ]*"//".*           { /* Handle comment lines - don't consume newline */ }
^[ ]*                 {
                        if (at_line_start) {
                          handle_indent(yyleng);
                          at_line_start = 0;
                          int token = process_queue();
                          if (token) return token;
                        }
                      }

":"[ \t]*\n           { 
                        at_line_start = 1; 
                        return COLON_NEWLINE; 
                      }

\n+                   { at_line_start = 1; return NEWLINE; }
[ \t]+                ;

"//".*                ;

"fun"                 {return FUN;}
(int|float|void|any|string|bool) {yylval.str = strdup(yytext); return TYPE;}
"while"               {return WHILE;}
"for"                 {return FOR;}
"if"                  {return IF;}
"else"                {return ELSE;}
"len"                 {return LEN;}
"return"              {return RETURN;}

"["                   {return LBRACKET;}
"]"                   {return RBRACKET;}
","                   {return COMMA;}
"("                   {return LPAREN;}
")"                   {return RPAREN;}
[0-9]+                {yylval.str = strdup(yytext); return NUMBER;}

":"                   {return COLON;}
";"                   {return SEMICOLON;}
"="                   {return ASSIGNMENT;}
"=="                  {return EQUALS;}
"<="                  {return SMALLEREQUALS;}
">="                  {return BIGGEREQUALS;}
"<"                   {return SMALLER;}
">"                   {return BIGGER;}
"++"                  {return INCREMENT;}
"--"                  {return DECREMENT;}
"**"                  {return EXPONENTIATION;}
"*"                   {return MULTIPLICATION;}
"/"                   {return DIVISION;}
"+"                   {return ADDITION;}
"-"                   {return SUBTRACTION;}

[a-zA-Z_][a-zA-Z0-9_]* {yylval.str = strdup(yytext); return ID;}
\"[^\"]*\"            {yylval.str = strdup(yytext); return STRING;}
\'[^\']*\'            {yylval.str = strdup(yytext); return STRING;}

.                     {fprintf(stderr, "Erro Léxico: Caractere inesperado '%s'\n", yytext); exit(1);}

<<EOF>>               {
                        handle_indent(0);
                        int token = process_queue();
                        if (token) return token;
                        if (!at_line_start) {
                            at_line_start = 1;
                            return NEWLINE;
                        }
                        return 0;
                      }
%%