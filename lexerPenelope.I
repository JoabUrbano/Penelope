%{
  #include <stdio.h>
  int current_indent = 0;
  int previous_indent = 0;

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

^\t+                  {int tabs = yyleng;handle_indent(tabs);}
^\n                   {handle_indent(0);}
[ \t]+                ;
(int|float|void|any|string|bool)      {printf("TYPE(%s)\n", yytext);}
\[                    {printf("LBRACKET\n");}
\]                    {printf("RBRACKET\n");}
,                     {printf("COMMA\n");}
\(                    {printf("LPAREN\n");}
\)                    {printf("RPAREN\n");}
[0-9]+                {printf("NUMBER(%s)\n",yytext);}
"while"               {printf("WHILE\n");}
"for"                 {printf("FOR\n");}
"if"                  {printf("IF\n");}
"else"                {printf("ELSE\n");}
"len"                 {printf("LEN\n");}
"return"              {printf("RETURN\n");}
":[ \t]*\n"           {printf("COLON_NEWLINE\n");} //não permite código seguido na mesma linha dos :
":"                   {printf("TYPYNG\n");}
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
[a-zA-Z][a-zA-Z0-9]*	{printf("ID(%s)\n",yytext);} /// \"[^\"]*\"|\'[^\']*\' {printf("STRING(%s)\n", yytext); }
\"                    {printf("DOUBLEQUOTES\n",yytext);} 
\'                    {printf("SINPLEQUOTES\n",yytext);}
.                     {printf("Invalid character.\n");}

%%

#ifndef yywrap
   int yywrap (void) {return 1;}
#endif

int main(void)
{
  yylex();  
  return 0;
}