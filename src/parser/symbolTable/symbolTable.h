#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "../../utils/hashMap/hashMap.h"

// Estruturas para gerenciamento de escopo
#define MAX_SCOPE_DEPTH 100

extern char* scopeStack[MAX_SCOPE_DEPTH];
extern int scopeTop;
extern HashMap symbolTable;
extern char *currentScope;

// Estruturas para gerenciamento de structs
typedef struct StructField {
    char* name;
    char* type;
    struct StructField* next;
} StructField;

typedef struct StructDefinition {
    char* name;
    StructField* fields;
    struct StructDefinition* next;
} StructDefinition;

extern StructDefinition* struct_definitions;

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

// Funções para gerenciamento de structs
void define_struct(const char* struct_name, StructField* fields);
StructDefinition* find_struct_definition(const char* struct_name);
StructField* find_struct_field(const char* struct_name, const char* field_name);
void add_struct_field(StructField** fields, const char* field_name, const char* field_type);
void free_struct_fields(StructField* fields);
void free_struct_definition(StructDefinition* def);

#endif // SYMBOL_TABLE_H
