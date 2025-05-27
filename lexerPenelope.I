%{
  #include <stdio.h>
  int current_indent = 0;
  int previous_indent = 0;
  int at_line_start = 1;

  void handle_indent(int count) {
    if (count > previous_indent) {
        printf("INDENT\n");
    } else if (count < previous_indent) {
        for (int i = 0; i < (previous_indent - count); i++) {
            printf("DEDENT\n");
        }
    }
    previous_indent = count;
  }  
%}

%%

^[ ]{4}*              {
                        if (at_line_start) {
                          int tabs = yyleng/4;
                          handle_indent(tabs);
                          at_line_start = 0;
                        }
                      }
\n                    {at_line_start = 1;}
[ \t]+                ; /* ignore whitespace */

"//".*                ; /* ignore comments */

"fun"                 {printf("FUN\n");}
(int|float|void|any|string|bool)      {printf("TYPE(%s)\n", yytext);}
"while"               {printf("WHILE\n");}
"for"                 {printf("FOR\n");}
"if"                  {printf("IF\n");}
"else"                {printf("ELSE\n");}
"len"                 {printf("LEN\n");}
"return"              {printf("RETURN\n");}

\[                    {printf("LBRACKET\n");}
\]                    {printf("RBRACKET\n");}
,                     {printf("COMMA\n");}
\(                    {printf("LPAREN\n");}
\)                    {printf("RPAREN\n");}
[0-9]+                {printf("NUMBER(%s)\n",yytext);}

:[ \t]*\n           {printf("COLON_NEWLINE\n"); at_line_start = 1;} 
":"                   {printf("COLON\n");}
";"                   {printf("SEMICOLON\n");}
"="                   {printf("ASSIGNMENT\n");}
"<="                  {printf("SMALLEREQUALS\n");}
">="                  {printf("BIGGEREQUALS\n");}
"<"                   {printf("SMALLER\n");}
">"                   {printf("BIGGER\n");}
"++"                  {printf("INCREMENT\n");}
"--"                  {printf("DECREMENT\n");}
"**"                  {printf("EXPONENTIATION\n");}
"*"                   {printf("MULTIPLICATION\n");}
"/"                   {printf("DIVISION\n");}
"+"                   {printf("ADDITION\n");}
"-"                   {printf("SUBTRACTION\n");}

[a-zA-Z][a-zA-Z0-9]*  {printf("ID(%s)\n",yytext);}
\"[^\"]*\"            {printf("STRING(%s)\n", yytext);}
\'[^\']*\'            {printf("STRING(%s)\n", yytext);}

.                     {printf("CHAR(%s)\n", yytext);} 
%%

#ifndef yywrap
   int yywrap (void) {return 1;}
#endif

int main(void)
{
  yylex();  
  return 0;
}