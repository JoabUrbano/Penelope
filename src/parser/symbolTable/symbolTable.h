#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "../../utils/hashMap/hashMap.h"

// Estruturas para gerenciamento de escopo
#define MAX_SCOPE_DEPTH 100

extern char* scopeStack[MAX_SCOPE_DEPTH];
extern int scopeTop;
extern HashMap symbolTable;
extern char *currentScope;

// Funções de gerenciamento de escopo
void push_scope(const char* scope_name);
void pop_scope();
Data* find_variable_in_current_scope(const char* name);
Data* find_variable_in_scopes(const char* name);

// Funções de compatibilidade de tipos
int are_types_compatible(const char* declaredType, const char* exprType);
char* get_array_element_type(const char* arrayType);

// Funções para gerenciamento de funções
void store_function(const char* func_name, const char* return_type);
char* get_function_return_type(const char* func_name);

#endif // SYMBOL_TABLE_H
