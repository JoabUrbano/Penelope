#ifndef PARSER_COMMON_H
#define PARSER_COMMON_H

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <errno.h>

// Project includes
#include "../utils/hashMap/hashMap.h"
#include "../utils/uniqueIdentifier/uniqueIdentifier.h"
#include "../structs/expression/expressionResult.h"
#include "../structs/lvalue/lvalueResult.h"

// Parser modules
#include "symbolTable.h"
#include "codeGenerator.h"
#include "semantics.h"
#include "expressionUtils.h"

// Function declarations that will be used in grammar rules
extern int yylineno;
extern int yylex();
extern void yyerror(const char* s);
extern FILE* yyin;

// Additional function declarations needed for parser
void free_expression_result(ExpressionResult* expr);
void free_lvalue_result(LValueResult* lval);

#endif // PARSER_COMMON_H
