#ifndef PARSER_COMMON_H
#define PARSER_COMMON_H

#include <stdio.h>
#include "symbolTable/symbolTable.h"
#include "codeGenerator/codeGenerator.h"
#include "semantics/semantics.h"
#include "grammarActions/grammarActions.h"
#include "expressionUtils/expressionUtils.h"
#include "../utils/hashMap/hashMap.h"
#include "../utils/uniqueIdentifier/uniqueIdentifier.h"

// Declarações externas do parser gerado
extern int yylineno;
extern int yylex();
extern int yyparse();
extern FILE* yyin;
extern void yyerror(const char* s);

// Variáveis globais comuns acessíveis em todos os módulos
extern int semantic_errors;
extern int syntax_errors;
extern char generated_code[];
extern int generate_code;

// Variáveis globais para gerenciamento de escopo e tabela de símbolos
extern HashMap symbolTable;
extern char* currentScope;
extern char* scopeStack[];
extern int scopeTop;
extern int exec_block;
extern int last_condition_result;
extern int current_loop_exit_label;

// Funções para gerenciamento de escopo
void push_scope(const char *scope);
void pop_scope();

// Funções para impressão da tabela de símbolos
void print_map(HashMap* map);
void free_map(HashMap* map);

#endif // PARSER_COMMON_H
