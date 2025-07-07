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
char *currentScope = NULL;
int semantic_errors = 0; 
int syntax_errors = 0; 


int are_types_compatible(const char* declaredType, const char* exprType) {
    if (strcmp(declaredType, exprType) == 0) return 1;
    if ((strcmp(declaredType, "float") == 0 && strcmp(exprType, "int") == 0) ||
      (strcmp(declaredType, "int") == 0 && strcmp(exprType, "float") == 0)) return 1;
    return 0;
}

// Função para extrair o tipo base de um tipo de array (ex: "int[]" -> "int")
char* get_array_element_type(const char* arrayType) {
    if (!arrayType || !strstr(arrayType, "[]")) {
        return NULL; // Não é um tipo de array
    }
    
    char* elementType = malloc(strlen(arrayType));
    strcpy(elementType, arrayType);
    
    // Remove o "[]" do final
    char* bracket = strstr(elementType, "[]");
    if (bracket) {
        *bracket = '\0';
    }
    
    return elementType;
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

Node* find_variable_in_current_scope(char *name) {
    if (currentScope == NULL) return NULL;
    
    char *fullKey = malloc(strlen(currentScope) + strlen(name) + 2);
    sprintf(fullKey, "%s#%s", currentScope, name);
    
    Node *node = find_node(&symbolTable, fullKey);
    free(fullKey);
    
    return node;
}

double evaluate_number(char *str) {
    return atof(str);
}

void print_value(double value) {
    printf("%.6g ", value);  // Imprime com espaço, sem quebra de linha, até 6 dígitos significativos
}

void print_string(char *str) {
    // Remove aspas de literais de string
    if (str && str[0] == '"' && str[strlen(str)-1] == '"') {
        str[strlen(str)-1] = '\0';  // Remove aspas finais
        printf("%s ", str + 1);   // Pula aspas iniciais, adiciona espaço
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
    #include "./structs/lvalue/lvalueResult.h"
}

%union {
    char *str;
    double num;
    ExpressionResult* exprResult;
    LValueResult* lvalueResult;
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

%type <lvalueResult> lvalue
%type <str> type list_expression
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
    FUN type ID LPAREN {
        // Cria o escopo da função antes de processar parâmetros
        char *scopeId = uniqueIdentifier();
        push_scope(scopeId);
    } list_param_opt RPAREN LBRACE list_stmt RBRACE {
        // Remove o escopo da função
        pop_scope();
    }
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
         if (find_variable_in_current_scope($3) != NULL) {
              semantic_error("Variável '%s' já declarada no escopo atual.", $3);
              YYABORT;
          }


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
        if (find_variable_in_current_scope($3) != NULL) {
              semantic_error("Variável '%s' já declarada no escopo atual.", $3);
              YYABORT;
        }
        
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
        } else if (strstr($1, "[]") != NULL) {
            // Tipos de array - armazena um valor padrão por enquanto
            newData.value.intVal = 0; // Valor temporário para arrays
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
        } else if (strcmp(newData.type, "string") == 0) {
            printf("%s\n", newData.value.strVal ? newData.value.strVal : "(null)");
        } else if (strstr(newData.type, "[]") != NULL) {
            printf("array\n");
        } else {
            printf("unknown\n");
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
    type COLON ID {
        // Adiciona o parâmetro à tabela de símbolos no escopo atual da função
        if (find_variable_in_current_scope($3) != NULL) {
            semantic_error("Parâmetro '%s' já declarado na função.", $3);
        } else {
            char *fullKey = malloc(strlen(currentScope) + strlen($3) + 2);
            sprintf(fullKey, "%s#%s", currentScope, $3);
            
            Data data;
            data.type = strdup($1);
            
            insert_node(&symbolTable, fullKey, data);
            free(fullKey);
        }
    }
    ;

return_stmt:
    RETURN expression { /* Poderia adicionar verificação de tipo de retorno aqui */ }
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
    ;

assign_stmt:
    lvalue ASSIGNMENT expression {
        // Verifica se a variável do lvalue foi declarada antes de atribuir
        if ($1->type == LVALUE_ARRAY_ACCESS) {
            // Acesso a array - verifica compatibilidade de tipos
            if (strcmp($1->elementType, $3->type) != 0) {
                // Permite conversão implícita de int para float
                if (!(strcmp($1->elementType, "float") == 0 && strcmp($3->type, "int") == 0)) {
                    semantic_error("Incompatibilidade de tipos: tentativa de atribuir '%s' a elemento de array do tipo '%s'.", 
                                   $3->type, $1->elementType);
                    YYABORT;
                }
            }
            // TODO: Implementar armazenamento real de valores em arrays
        } else if ($1->type == LVALUE_VAR) {
            if (find_variable_in_scopes($1->varName) == NULL) {
                semantic_error("Variável '%s' não declarada.", $1->varName);
                YYABORT;
            } else {
                // store_variable_value($1->varName, $3);
            }
        }
        free_lvalue_result($1);
    }
    | lvalue INCREMENT {
        if ($1->type == LVALUE_ARRAY_ACCESS) {
            // Incremento em elemento de array
            if (strcmp($1->elementType, "int") != 0 && strcmp($1->elementType, "float") != 0) {
                semantic_error("Não é possível usar o operador ++ para elementos de array do tipo '%s'.", $1->elementType);
                YYABORT;
            }
            // TODO: Implementar incremento real em elementos de array
        } else if ($1->type == LVALUE_VAR) {
            Node* node = find_variable_in_scopes($1->varName);

            if (node == NULL) {
                semantic_error("Variável '%s' não declarada.", $1->varName);
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
        free_lvalue_result($1);
    }
    | lvalue DECREMENT {
        if ($1->type == LVALUE_ARRAY_ACCESS) {
            // Decremento em elemento de array
            if (strcmp($1->elementType, "int") != 0 && strcmp($1->elementType, "float") != 0) {
                semantic_error("Não é possível usar o operador -- para elementos de array do tipo '%s'.", $1->elementType);
                YYABORT;
            }
            // TODO: Implementar decremento real em elementos de array
        } else if ($1->type == LVALUE_VAR) {
             if (find_variable_in_scopes($1->varName) == NULL) {
                semantic_error("Variável '%s' não declarada.", $1->varName);
                YYABORT;
            } else {
                // double current = get_variable_value($1->varName);
                // store_variable_value($1->varName, current - 1.0);
            }
        }
        free_lvalue_result($1);
    }
    ;

lvalue:
    ID { 
        $$ = create_lvalue_var($1);
    }
    | lvalue LBRACKET expression RBRACKET { 
        // Para acesso a array, verifica se o lvalue é um array e retorna o tipo do elemento
        
        // Se o lvalue já é um array_access, não podemos fazer double indexing nesta implementação
        if ($1->type == LVALUE_ARRAY_ACCESS) {
            semantic_error("Acesso multidimensional a arrays não é suportado nesta versão.");
            YYABORT;
        }
        
        Node* arrayNode = find_variable_in_scopes($1->varName);
        if (!arrayNode) {
            semantic_error("Variável '%s' não declarada.", $1->varName);
            YYABORT;
        }
        
        if (!strstr(arrayNode->value.type, "[]")) {
            semantic_error("Tentativa de indexar uma variável que não é um array: '%s'.", $1->varName);
            YYABORT;
        }
        
        // Verifica se o índice é um tipo válido (int)
        if (strcmp($3->type, "int") != 0) {
            semantic_error("Índice de array deve ser do tipo int, mas foi '%s'.", $3->type);
            YYABORT;
        }
        
        // Cria um resultado de array access com o tipo do elemento
        char* elementType = get_array_element_type(arrayNode->value.type);
        $$ = create_lvalue_array_access($1->varName, elementType);
        free(elementType);
        free_lvalue_result($1); // Libera o lvalue anterior
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
        
        if ($1->type == LVALUE_ARRAY_ACCESS) {
            // Manipula acesso a array - usa o tipo do elemento especificado
            result->type = strdup($1->elementType);
            
            // Define valores padrão baseados no tipo do elemento
            if (strcmp($1->elementType, "int") == 0) {
                result->intVal = 0;
            } else if (strcmp($1->elementType, "float") == 0) {
                result->doubleVal = 0.0;
            } else if (strcmp($1->elementType, "bool") == 0) {
                result->intVal = 0; // false
            } else if (strcmp($1->elementType, "string") == 0) {
                result->strVal = strdup("");
            } else {
                semantic_error("Tipo de elemento de array não suportado: '%s'.", $1->elementType);
                free(result);
                free_lvalue_result($1);
                YYABORT;
            }
        } else if ($1->type == LVALUE_VAR) {
            Node* varNode = find_variable_in_scopes($1->varName);
            
            if (!varNode) {
                semantic_error("Variável '%s' não declarada.", $1->varName);
                free(result);
                free_lvalue_result($1);
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
                result->strVal = strdup(varNode->value.value.strVal);
            } else if (strstr(varNode->value.type, "[]") != NULL) {
                // Tipos de array são suportados - define valor padrão por enquanto
                result->intVal = 0; // Arrays avaliam para 0 por enquanto (valor temporário)
            } else {
                semantic_error("Tipo '%s' não suportado em expressões.", varNode->value.type);
                free(result);
                free_lvalue_result($1);
                YYABORT;
            }
            }
        }

        free_lvalue_result($1);
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
    | expression SMALLER expression {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        res->type = strdup("bool");
        
        if (!are_types_compatible($1->type, $3->type) || strcmp($1->type, "string") == 0) {
             semantic_error("Operador '<' inválido entre os tipos %s e %s.", $1->type, $3->type);
             free(res);
             YYABORT;
        }

        double left = (strcmp($1->type, "int") == 0) ? (double)$1->intVal : $1->doubleVal;
        double right = (strcmp($3->type, "int") == 0) ? (double)$3->intVal : $3->doubleVal;
        
        res->intVal = (left < right);
        
        free_expression_result($1);
        free_expression_result($3);
        $$ = res;
    }
    | expression BIGGER expression {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        res->type = strdup("bool");
        
        if (!are_types_compatible($1->type, $3->type) || strcmp($1->type, "string") == 0) {
             semantic_error("Operador '>' inválido entre os tipos %s e %s.", $1->type, $3->type);
             free(res);
             YYABORT;
        }

        double left = (strcmp($1->type, "int") == 0) ? (double)$1->intVal : $1->doubleVal;
        double right = (strcmp($3->type, "int") == 0) ? (double)$3->intVal : $3->doubleVal;
        
        res->intVal = (left > right);
        
        free_expression_result($1);
        free_expression_result($3);
        $$ = res;
    }
    | expression SMALLEREQUALS expression {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        res->type = strdup("bool");
        
        if (!are_types_compatible($1->type, $3->type) || strcmp($1->type, "string") == 0) {
             semantic_error("Operador '<=' inválido entre os tipos %s e %s.", $1->type, $3->type);
             free(res);
             YYABORT;
        }

        double left = (strcmp($1->type, "int") == 0) ? (double)$1->intVal : $1->doubleVal;
        double right = (strcmp($3->type, "int") == 0) ? (double)$3->intVal : $3->doubleVal;
        
        res->intVal = (left <= right);
        
        free_expression_result($1);
        free_expression_result($3);
        $$ = res;
    }
    | expression BIGGEREQUALS expression {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        res->type = strdup("bool");
        
        if (!are_types_compatible($1->type, $3->type) || strcmp($1->type, "string") == 0) {
             semantic_error("Operador '>=' inválido entre os tipos %s e %s.", $1->type, $3->type);
             free(res);
             YYABORT;
        }

        double left = (strcmp($1->type, "int") == 0) ? (double)$1->intVal : $1->doubleVal;
        double right = (strcmp($3->type, "int") == 0) ? (double)$3->intVal : $3->doubleVal;
        
        res->intVal = (left >= right);
        
        free_expression_result($1);
        free_expression_result($3);
        $$ = res;
    }
    | expression EQUALS expression {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        res->type = strdup("bool");
        int result = 0;

        // Caso 1: Comparação de strings
        if (strcmp($1->type, "string") == 0 && strcmp($3->type, "string") == 0) {
            result = (strcmp($1->strVal, $3->strVal) == 0);
        } 
        
        // Caso 2: Comparação de tipos numéricos compatíveis
        else if (are_types_compatible($1->type, $3->type)) {
            double left = (strcmp($1->type, "int") == 0) ? (double)$1->intVal : $1->doubleVal;
            double right = (strcmp($3->type, "int") == 0) ? (double)$3->intVal : $3->doubleVal;
            result = (left == right);
        } 
        // Caso 3: Tipos incompatíveis
        else {
            semantic_error("Operador '==' inválido entre os tipos %s e %s.", $1->type, $3->type);
            free(res);
            YYABORT;
        }

        res->intVal = result;
        free_expression_result($1);
        free_expression_result($3);
        $$ = res;
    }
    | SUBTRACTION expression %prec UMINUS {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        if (strcmp($2->type, "int") == 0) {
            res->type = strdup("int");
            res->intVal = -$2->intVal;
        } else if (strcmp($2->type, "float") == 0) {
            res->type = strdup("float");
            res->doubleVal = -$2->doubleVal;
        } else {
            semantic_error("Operador unário '-' inválido para o tipo %s.", $2->type);
            free(res);
            YYABORT;
        }
        free_expression_result($2);
        $$ = res;
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
          // Determina o tipo do array baseado no primeiro elemento
          char* arrayType = malloc(strlen($2) + 3);
          sprintf(arrayType, "%s[]", $2);
          res->type = arrayType;
          res->intVal = 0; // Valor temporário para literais de array
          $$ = res;
      }
    ;


list_expression:
    expression                                     { $$ = strdup($1->type); }
    | list_expression COMMA expression             { $$ = $1; } // Mantém o tipo do primeiro elemento
    ;

arg_list_opt:
                                                   { /* vazio */ }
    | arg_list                                     { /* lista de argumentos */ }
    ;

arg_list:
    expression                                     { /* argumento único */ }
    | arg_list COMMA expression                    { /* múltiplos argumentos */ }
    ;

%%

void yyerror(const char* s) {
    extern int yylineno;
    fprintf(stderr, "Erro de Sintaxe: erro de sintaxe na linha %d\n", yylineno);
    syntax_errors++;
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

    int parse_result = yyparse();
    
    if (parse_result == 0 && semantic_errors == 0) {
        printf("Análise concluída com sucesso. A sintaxe e a semântica estão corretas!\n");
    } else {
        printf("Falha na análise. Foram encontrados ");
        
        if (semantic_errors > 0 && syntax_errors > 0) {
            printf("%d erros semânticos e %d erros de sintaxe.\n", semantic_errors, syntax_errors);
        } else if (semantic_errors > 0) {
            printf("%d erros semânticos.\n", semantic_errors);
        } else if (syntax_errors > 0) {
            printf("%d erros de sintaxe.\n", syntax_errors);
        }
    }

    print_map(&symbolTable);
    
    // Libera o escopo global
    pop_scope(); 

    free_map(&symbolTable);

    return (parse_result != 0 || semantic_errors > 0) ? 1 : 0;
}
