#include "parser_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Este arquivo contém funções comuns do parser que não estão em outros módulos
// As funções free_expression_result e free_lvalue_result estão implementadas
// nos respectivos arquivos structs/expression/expressionResult.c e structs/lvalue/lvalueResult.c

// A função yyerror deve ficar no arquivo principal do parser (parserPenelope.y)
// para que o Bison possa encontrá-la corretamente
