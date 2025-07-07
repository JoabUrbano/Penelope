#ifndef PARSER_COMMON_H
#define PARSER_COMMON_H

// Inclusões padrão
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <errno.h>

// Inclusões do projeto
#include "../../utils/hashMap/hashMap.h"
#include "../../utils/uniqueIdentifier/uniqueIdentifier.h"
#include "../../structs/expression/expressionResult.h"
#include "../../structs/lvalue/lvalueResult.h"

// Módulos do parser
#include "../symbolTable/symbolTable.h"
#include "../codeGenerator/codeGenerator.h"
#include "../semantics/semantics.h"
#include "../expressionUtils/expressionUtils.h"
#include "../grammarActions/grammarActions.h"

// Declarações de funções que serão usadas nas regras da gramática
extern int yylineno;
extern int yylex();
extern void yyerror(const char* s);
extern FILE* yyin;

// Declarações de funções adicionais necessárias para o parser
void free_expression_result(ExpressionResult* expr);
void free_lvalue_result(LValueResult* lval);

#endif // PARSER_COMMON_H
