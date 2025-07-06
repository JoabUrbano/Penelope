%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

#include "./utils/hashMap/hashMap.h"
#include "./utils/uniqueIdentifier/uniqueIdentifier.h"

#define MAX_SCOPE_DEPTH 100
char* scopeStack[MAX_SCOPE_DEPTH];
int scopeTop = -1;

HashMap symbolTable = { NULL };
HashMap valueTable = { NULL };  // Store actual values
char *currentScope = NULL;
int semantic_errors = 0; 


int are_types_compatible(const char* declaredType, const char* exprType) {
    if (strcmp(declaredType, exprType) == 0) return 1;
    // if ((strcmp(declaredType, "float") == 0 && strcmp(exprType, "int") == 0) ||
    //   (strcmp(declaredType, "int") == 0 && strcmp(exprType, "float") == 0)) return 1;
    return 0;
}


void semantic_error(const char* format, ...) {
    extern int yylineno;
    fprintf(stderr, "Erro Semântico na linha %d: ", yylineno);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);  // Imprime a mensagem formatada
    va_end(args);

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

/* Inclua o header aqui, *fora* do bloco %{...%}, para que o Bison leia a definição do tipo ANTES do %union */
%code requires {
    #include "./structs/expression/expressionResult.h"
}

%union {
    char *str;
    double num;
    ExpressionResult* exprResult;
}


%token <num> BOOL
%token <str> ID TYPE STRING
%token <num> INT FLOAT

%token FUN WHILE FOR IF ELSE LEN PRINT RETURN
%token LBRACKET RBRACKET COMMA LPAREN RPAREN COLON SEMICOLON NEWLINE
%token ASSIGNMENT EQUALS SMALLEREQUALS BIGGEREQUALS SMALLER BIGGER
%token INCREMENT DECREMENT EXPONENTIATION MULTIPLICATION DIVISION ADDITION SUBTRACTION
%token LBRACE RBRACE

%define parse.trace

%type <str> lvalue type list_expression
%type <exprResult> expression

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
      | type COLON ID {
        // Cria chave completa com escopo: "escopo#variavel"
        char *fullKey = malloc(strlen(currentScope) + strlen($3) + 2);
        
        if (!fullKey) {
            semantic_error("Erro de alocação de memória para chave do símbolo.\n");

            YYABORT;
        }

        sprintf(fullKey, "%s#%s", currentScope, $3);

        Data data;

        data.type = strdup($1);

        insert_node(&symbolTable, fullKey, data);

        free(fullKey);
    }
    | type COLON ID ASSIGNMENT expression {
        char *fullKey = malloc(strlen(currentScope) + strlen($3) + 2);
        if (!fullKey) {
            semantic_error("Erro de alocação de memória para chave.\n");
            YYABORT;
        }
        sprintf(fullKey, "%s#%s", currentScope, $3);

        // CORREÇÃO: $1 é uma string, não struct com campo .type
        if (!are_types_compatible($1, $5->type)) {
            free(fullKey);
            semantic_error("Tipo incompatível: a variável de tipo %s não pode receber o tipo %s\n", $1, $5->type);
            YYABORT;
        }

        Data newData;
        newData.type = strdup($1);
        
        if (strcmp($1, "int") == 0) {
            newData.value.intVal = $5->intVal;
        } else if (strcmp($1, "float") == 0) {
            newData.value.doubleVal = $5->doubleVal;
        } else if (strcmp($1, "bool") == 0) {
            newData.value.intVal = $5->intVal;
        } else if (strcmp($1, "string") == 0) {
            newData.value.strVal = $5->strVal;
        } else {
            free(fullKey);
            semantic_error("Tipo '%s' não suportado para atribuição.\n", $1);
            YYABORT;
        }

        printf("Inserido no symbolTable: %s com tipo %s e valor ", fullKey, newData.type);
        if (strcmp(newData.type, "int") == 0) {
            printf("%d\n", newData.value.intVal);
        } else if (strcmp(newData.type, "float") == 0) {
            printf("%f\n", newData.value.doubleVal);
        } else if (strcmp(newData.type, "bool") == 0) {
            printf("%d\n", newData.value.intVal);
        }
        
        insert_node(&symbolTable, fullKey, newData);

        free(newData.type); // insert_node já faz cópia
        free(fullKey);
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
        if ($1 != NULL) {
            if (strcmp($1->type, "float") == 0) {
                printf("%f\n", $1->doubleVal);
            } else if (strcmp($1->type, "string") == 0) {
                printf("%s\n", $1->strVal);
            } else if (strcmp($1->type, "int") == 0) {
                printf("%d\n", $1->intVal);
            }
        }
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
                YYABORT;
            } else {
                // store_variable_value($1, $3);
            }
        }
    }
    | lvalue INCREMENT {
        if ($1 && strcmp($1, "array_access") != 0) {
            Node* node = find_variable_in_scopes($1);

            if (node == NULL) {
                semantic_error("Variável '%s' não declarada.", $1);
                YYABORT;
            } else {
                char* type = node->value.type;

                if (strcmp(type, "int") != 0 && strcmp(type, "float") != 0) {
                    semantic_error("Não é possível usar o operador ++ para tipos que não sejam int e float\n");
                    YYABORT;
                } else if (strcmp(type, "int") == 0) {
                    node->value.value.intVal += 1;
                } else if (strcmp(type, "float") == 0) {
                    node->value.value.doubleVal += 1.0;
                }
            }
        }
    }
    | lvalue DECREMENT {
        if ($1 && strcmp($1, "array_access") != 0) {
             if (find_variable_in_scopes($1) == NULL) {
                semantic_error("Variável '%s' não declarada.", $1);
                YYABORT;
            } else {
                // double current = get_variable_value($1);
                // store_variable_value($1, current - 1.0);
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
    BOOL {
        ExpressionResult* result = malloc(sizeof(ExpressionResult));
        result->type = strdup("bool");
        result->intVal = $1;
        $$ = result;
    }
    | INT {
        ExpressionResult* result = malloc(sizeof(ExpressionResult));
        result->type = strdup("int");
        result->intVal = $1;
        $$ = result;
    }
    | FLOAT {
        ExpressionResult* result = malloc(sizeof(ExpressionResult));
        result->type = strdup("float");
        result->doubleVal = $1;
        $$ = result;
    }
    | STRING {
          ExpressionResult* result = malloc(sizeof(ExpressionResult));
          result->type = strdup("string");
          result->strVal = strdup($1);
          $$ = result;
      }
    | lvalue {
        ExpressionResult* result = malloc(sizeof(ExpressionResult));
        Node* varNode = find_variable_in_scopes($1);
        
        if (!varNode) {
            semantic_error("Variável '%s' não declarada.", $1);
            YYABORT;
        } else {
            result->type = strdup(varNode->value.type);

            if (strcmp(varNode->value.type, "int") == 0) {
                result->intVal = varNode->value.value.intVal;
            } else if (strcmp(varNode->value.type, "float") == 0) {
                result->doubleVal = varNode->value.value.doubleVal;
            } else if (strcmp(varNode->value.type, "bool") == 0) {
                result->intVal = varNode->value.value.intVal;
            } else if (strcmp(varNode->value.type, "string") == 0) {
                result->strVal = varNode->value.value.strVal;
            } else {
                semantic_error("Tipo '%s' não suportado em expressões.", varNode->value.type);
                YYABORT;
            }
        }

        $$ = result;
    }
    | LPAREN expression RPAREN {
          $$ = $2;
      }
    | expression ADDITION expression {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        
        if (strcmp($1->type, "float") == 0 || strcmp($3->type, "float") == 0) {
            res->type = strdup("float");

            double leftVal = (strcmp($1->type, "int") == 0) ? (double)$1->intVal : $1->doubleVal;
            double rightVal = (strcmp($3->type, "int") == 0) ? (double)$3->intVal : $3->doubleVal;

            res->doubleVal = leftVal + rightVal;

        } else if (strcmp($1->type, "int") == 0 && strcmp($3->type, "int") == 0) {
            res->type = strdup("int");
            res->intVal = $1->intVal + $3->intVal;
        } else {
            // Caso tipos inesperados, força float como fallback
            res->type = strdup("float");
            res->doubleVal = 0.0;
            // Poderia emitir erro aqui, se quiser
        }

        free_expression_result($1);
        free_expression_result($3);

        $$ = res;
    }
    | expression SUBTRACTION expression {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        
        if (strcmp($1->type, "float") == 0 || strcmp($3->type, "float") == 0) {
            res->type = strdup("float");

            double leftVal = (strcmp($1->type, "int") == 0) ? (double)$1->intVal : $1->doubleVal;
            double rightVal = (strcmp($3->type, "int") == 0) ? (double)$3->intVal : $3->doubleVal;

            res->doubleVal = leftVal - rightVal;

        } else if (strcmp($1->type, "int") == 0 && strcmp($3->type, "int") == 0) {
            res->type = strdup("int");
            res->intVal = $1->intVal - $3->intVal;
        } else {
            // Caso tipos inesperados, força float como fallback
            res->type = strdup("float");
            res->doubleVal = 0.0;
            // Poderia emitir erro aqui, se quiser
        }

        free_expression_result($1);
        free_expression_result($3);

        $$ = res;
      }
    | expression MULTIPLICATION expression {
          ExpressionResult* res = malloc(sizeof(ExpressionResult));
          res->type = strdup("float");
          res->doubleVal = $1->doubleVal * $3->doubleVal;
          $$ = res;
          free_expression_result($1);
          free_expression_result($3);
      }
    | expression DIVISION expression {
          ExpressionResult* res = malloc(sizeof(ExpressionResult));
          res->type = strdup("float");
          if ($3->doubleVal == 0) {
              semantic_error("Divisão por zero.", "");
              YYABORT;
          } else {
              res->doubleVal = $1->doubleVal / $3->doubleVal;
          }
          $$ = res;
          free_expression_result($1);
          free_expression_result($3);
      }
    | expression EXPONENTIATION expression {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        res->type = strdup("float");

        double left = strcmp($1->type, "int") == 0 ? (double)$1->intVal : $1->doubleVal;
        double right = strcmp($3->type, "int") == 0 ? (double)$3->intVal : $3->doubleVal;

        res->doubleVal = power_operation(left, right);

        $$ = res;
        free_expression_result($1);
        free_expression_result($3);
      }
    // Comparações: retornam 1.0 ou 0.0 (float)
    | expression SMALLER expression {
          ExpressionResult* res = malloc(sizeof(ExpressionResult));
          res->type = strdup("float");
          res->doubleVal = ($1->doubleVal < $3->doubleVal) ? 1.0 : 0.0;
          $$ = res;
          free_expression_result($1);
          free_expression_result($3);
      }
    | expression BIGGER expression {
          ExpressionResult* res = malloc(sizeof(ExpressionResult));
          res->type = strdup("float");
          res->doubleVal = ($1->doubleVal > $3->doubleVal) ? 1.0 : 0.0;
          $$ = res;
          free_expression_result($1);
          free_expression_result($3);
      }
    | expression SMALLEREQUALS expression {
          ExpressionResult* res = malloc(sizeof(ExpressionResult));
          res->type = strdup("float");
          res->doubleVal = ($1->doubleVal <= $3->doubleVal) ? 1.0 : 0.0;
          $$ = res;
          free_expression_result($1);
          free_expression_result($3);
      }
    | expression BIGGEREQUALS expression {
          ExpressionResult* res = malloc(sizeof(ExpressionResult));
          res->type = strdup("float");
          res->doubleVal = ($1->doubleVal >= $3->doubleVal) ? 1.0 : 0.0;
          $$ = res;
          free_expression_result($1);
          free_expression_result($3);
      }
    | expression EQUALS expression {
          ExpressionResult* res = malloc(sizeof(ExpressionResult));
          res->type = strdup("float");
          res->doubleVal = (strcmp($1->type, $3->type) == 0 && (
              (strcmp($1->type, "float") == 0 && $1->doubleVal == $3->doubleVal) ||
              (strcmp($1->type, "string") == 0 && strcmp($1->strVal, $3->strVal) == 0)
          )) ? 1.0 : 0.0;
          $$ = res;
          free_expression_result($1);
          free_expression_result($3);
      }
    | SUBTRACTION expression %prec UMINUS {
          ExpressionResult* res = malloc(sizeof(ExpressionResult));
          res->type = strdup("float");
          res->doubleVal = -$2->doubleVal;
          $$ = res;
          free_expression_result($2);
      }
    | ID LPAREN arg_list_opt RPAREN {
          ExpressionResult* res = malloc(sizeof(ExpressionResult));
          res->type = strdup("float");
          res->doubleVal = 0.0; // Função não implementada ainda
          $$ = res;
      }
    | LEN LPAREN expression RPAREN {
          ExpressionResult* res = malloc(sizeof(ExpressionResult));
          res->type = strdup("int");
          res->doubleVal = 0.0; // len não implementado ainda
          $$ = res;
      }
    | LBRACKET list_expression RBRACKET {
          ExpressionResult* res = malloc(sizeof(ExpressionResult));
          res->type = strdup("float");
          res->doubleVal = 0.0; // arrays não implementados
          $$ = res;
      }
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
