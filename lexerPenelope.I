%{
#include <stdio.h>
#include <string.h>
#include "parser.tab.h"
%}

%option noyywrap
%option yylineno

%%

"fun"                { return FUN; }
(int|float|void|any|string|bool) { yylval.str = strdup(yytext); return TYPE; }
"true"|"false"       { yylval.str = strdup(yytext); return ID; }
"while"              { return WHILE; }
"for"                { return FOR; }
"if"                 { return IF; }
"else"               { return ELSE; }
"len"                { return LEN; }
"print"              { return PRINT; }
"return"             { return RETURN; }

"{"                  { return LBRACE; }
"}"                  { return RBRACE; }
"["                  { return LBRACKET; }
"]"                  { return RBRACKET; }
","                  { return COMMA; }
"("                  { return LPAREN; }
")"                  { return RPAREN; }
":"                  { return COLON; }
";"                  { return SEMICOLON; }

"="                  { return ASSIGNMENT; }
"=="                 { return EQUALS; }
"<="                 { return SMALLEREQUALS; }
">="                 { return BIGGEREQUALS; }
"<"                  { return SMALLER; }
">"                  { return BIGGER; }
"++"                 { return INCREMENT; }
"--"                 { return DECREMENT; }
"**"                 { return EXPONENTIATION; }
"*"                  { return MULTIPLICATION; }
"/"                  { return DIVISION; }
"+"                  { return ADDITION; }
"-"                  { return SUBTRACTION; }

[0-9]+\.[0-9]+       { yylval.num = atof(yytext); return NUMBER; }
[0-9]+               { yylval.num = atof(yytext); return NUMBER; }

[a-zA-Z_][a-zA-Z0-9_]* { yylval.str = strdup(yytext); return ID; }

\"([^\\\"]|\\.)*\"   { yylval.str = strdup(yytext); return STRING; }
\'[^\']*\'           { yylval.str = strdup(yytext); return STRING; }

[ \t]+               ;  // ignora espaços e tabs
\n+                  { return NEWLINE; }

"//".*               ;  // ignora comentários de linha

.                    { fprintf(stderr, "Erro Léxico: Caractere inesperado '%s'\n", yytext); exit(1); }

%%

