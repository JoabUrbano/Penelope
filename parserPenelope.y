%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "./utils/hashMap/hashMap.h"
#include "./utils/uniqueIdentifier/uniqueIdentifier.h"

HashMap symbolTable = { NULL };
HashMap valueTable = { NULL };  // Store actual values
char *currentScope = NULL;

double evaluate_number(char *str) {
    return atof(str);
}

void print_value(double value) {
    printf("%.2f\n", value);
}

void print_string(char *str) {
    // Remove quotes from string literals
    if (str && str[0] == '"' && str[strlen(str)-1] == '"') {
        str[strlen(str)-1] = '\0';  // Remove ending quote
        printf("%s\n", str + 1);   // Skip beginning quote
    } else if (str) {
        printf("%s\n", str);
    }
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
    char *fullKey = malloc(strlen(currentScope) + strlen(name) + 2);
    sprintf(fullKey, "%s#%s", currentScope, name);
    
    Node *node = find_node(&valueTable, fullKey);
    free(fullKey);
    
    if (node) {
        return atof(node->value);
    }
    return 0.0; // Default value if not found
}

extern int yylex();
extern int yyparse();
extern FILE* yyin;
void yyerror(const char* s);
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
    FUN type ID LPAREN list_param_opt RPAREN LBRACE {
        if (currentScope) free(currentScope);
        currentScope = uniqueIdentifier();
    } list_stmt RBRACE

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
    WHILE LPAREN expression RPAREN LBRACE list_stmt RBRACE
    ;

if_stmt:
    IF LPAREN expression RPAREN LBRACE list_stmt RBRACE %prec LOWER_THAN_ELSE
    | IF LPAREN expression RPAREN LBRACE list_stmt RBRACE ELSE LBRACE list_stmt RBRACE
    ;

for_stmt:
    FOR LPAREN for_init SEMICOLON expression SEMICOLON assign_stmt RPAREN LBRACE list_stmt RBRACE
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
    PRINT LPAREN expression RPAREN {
        print_value($3);
    }
    | PRINT LPAREN STRING RPAREN {
        print_string($3);
    }
    ;

assign_stmt:
    ID ASSIGNMENT expression {
        store_variable_value($1, $3);
    }
    | ID INCREMENT {
        double current = get_variable_value($1);
        store_variable_value($1, current + 1.0);
    }
    | ID DECREMENT {
        double current = get_variable_value($1);
        store_variable_value($1, current - 1.0);
    }
    ;

lvalue:
    ID
    | lvalue LBRACKET expression RBRACKET
    ;

expression:
    NUMBER                                         { $$ = $1; }
    | ID                                           { $$ = get_variable_value($1); }
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
    free_map(&valueTable);

    return 0;
}
