#include "symbolTable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Variáveis globais para gerenciamento de escopo
char* scopeStack[MAX_SCOPE_DEPTH];
int scopeTop = -1;
HashMap symbolTable = { NULL };
char *currentScope = NULL;

// Armazenamento global de definições de struct
StructDefinition* struct_definitions = NULL;

// Função para verificar compatibilidade entre tipos
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
    
    char* elementType = malloc(strlen(arrayType) + 1);
    strcpy(elementType, arrayType);
    
    // Remove o primeiro "[]" encontrado
    char* bracket = strstr(elementType, "[]");
    if (bracket) {
        // Move o resto da string para a esquerda, removendo "[]"
        memmove(bracket, bracket + 2, strlen(bracket + 2) + 1);
    }
    
    return elementType;
}

void push_scope(const char* scope_name) {
    if (scopeTop < MAX_SCOPE_DEPTH - 1) {
        scopeStack[++scopeTop] = strdup(scope_name);
        currentScope = scopeStack[scopeTop];
    } else {
        fprintf(stderr, "Erro: Estouro da pilha de escopos\n");
    }
}

void pop_scope() {
    if (scopeTop >= 0) {
        free(scopeStack[scopeTop]);
        scopeStack[scopeTop--] = NULL;
        currentScope = (scopeTop >= 0) ? scopeStack[scopeTop] : NULL;
    } else {
        fprintf(stderr, "Erro: Pilha de escopos vazia\n");
    }
}

Data* find_variable_in_current_scope(const char* name) {
    if (currentScope == NULL) return NULL;
    
    char fullKey[256];
    sprintf(fullKey, "%s#%s", currentScope, name);
    
    Node* result = find_node(&symbolTable, fullKey);
    return result ? &result->value : NULL;
}

Data* find_variable_in_scopes(const char* name) {
    // Procura primeiro no escopo atual, depois nos anteriores
    for (int i = scopeTop; i >= 0; i--) {
        char fullKey[256];
        sprintf(fullKey, "%s#%s", scopeStack[i], name);
        
        Node* result = find_node(&symbolTable, fullKey);
        if (result) {
            return &result->value;
        }
    }
    return NULL;
}

// Função para armazenar informações de função na tabela de símbolos
void store_function(const char* func_name, const char* return_type) {
    if (!func_name || !return_type) return;
    
    char fullKey[256];
    sprintf(fullKey, "function#%s", func_name);
    
    Data data;
    data.type = strdup(return_type);
    data.value.intVal = 0;
    data.value.doubleVal = 0.0;
    data.value.strVal = NULL;
    
    insert_node(&symbolTable, fullKey, data);
}

// Função para obter o tipo de retorno de uma função
char* get_function_return_type(const char* func_name) {
    if (!func_name) return NULL;
    
    char fullKey[256];
    sprintf(fullKey, "function#%s", func_name);
    
    Node* result = find_node(&symbolTable, fullKey);
    if (result && result->value.type) {
        return strdup(result->value.type);
    }
    
    return NULL;
}

// ========== FUNÇÕES DE GERENCIAMENTO DE STRUCT ==========

void add_struct_field(StructField** fields, const char* field_name, const char* field_type) {
    StructField* new_field = malloc(sizeof(StructField));
    new_field->name = strdup(field_name);
    new_field->type = strdup(field_type);
    new_field->next = *fields;
    *fields = new_field;
}

void define_struct(const char* struct_name, StructField* fields) {
    StructDefinition* new_def = malloc(sizeof(StructDefinition));
    new_def->name = strdup(struct_name);
    new_def->fields = fields;
    new_def->next = struct_definitions;
    struct_definitions = new_def;
}

StructDefinition* find_struct_definition(const char* struct_name) {
    StructDefinition* current = struct_definitions;
    while (current) {
        if (strcmp(current->name, struct_name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

StructField* find_struct_field(const char* struct_name, const char* field_name) {
    StructDefinition* struct_def = find_struct_definition(struct_name);
    if (!struct_def) return NULL;
    
    StructField* current = struct_def->fields;
    while (current) {
        if (strcmp(current->name, field_name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void free_struct_fields(StructField* fields) {
    while (fields) {
        StructField* temp = fields;
        fields = fields->next;
        free(temp->name);
        free(temp->type);
        free(temp);
    }
}

void free_struct_definition(StructDefinition* def) {
    if (def) {
        free(def->name);
        free_struct_fields(def->fields);
        free(def);
    }
}

// ========== FUNÇÕES DE GERENCIAMENTO DE PARÂMETROS DE FUNÇÃO ==========

// HashMap para armazenar informações de parâmetros por função
HashMap function_params_map;
static int function_params_initialized = 0;

void init_function_params() {
    if (!function_params_initialized) {
        function_params_map.nodes = NULL;
        function_params_initialized = 1;
    }
}

// Função para armazenar parâmetros de função
void store_function_parameter(const char* func_name, const char* param_name, const char* param_type) {
    if (!func_name || !param_name || !param_type) return;
    
    // Debug: Print parameter information
    
    
    init_function_params();
    
    // Cria nova informação de parâmetro
    FunctionParamInfo* new_param = malloc(sizeof(FunctionParamInfo));
    new_param->name = strdup(param_name);
    new_param->type = strdup(param_type);
    new_param->next = NULL;
    
    // Procura se já existe lista de parâmetros para esta função
    Node* existing = find_node(&function_params_map, func_name);
    if (existing) {
        // Adiciona ao início da lista
        new_param->next = (FunctionParamInfo*)existing->value.value.strVal;
    }
    
    // Armazena na tabela
    Data data;
    data.type = strdup("function_params");
    data.value.strVal = (char*)new_param;
    
    insert_node(&function_params_map, func_name, data);
}

int is_function_parameter_reference(const char* func_name, int param_index) {
    init_function_params();
    
    Node* result = find_node(&function_params_map, func_name);
    if (!result) {
        printf("DEBUG: No parameters found for function %s\n", func_name);
        return 0;
    }
    
    FunctionParamInfo* params = (FunctionParamInfo*)result->value.value.strVal;
    printf("DEBUG: Found parameters for function %s, params pointer: %p\n", func_name, (void*)params);
    
    // Contar parâmetros (armazenados em ordem inversa)
    int count = 0;
    FunctionParamInfo* current = params;
    while (current) {
        printf("DEBUG: Parameter %d: %s (type: %s)\n", count, current->name, current->type);
        count++;
        current = current->next;
    }
    
    printf("DEBUG: Total parameters: %d, looking for index: %d\n", count, param_index);
    
    // Acessar parâmetro no índice correto
    current = params;
    for (int i = 0; i < count - param_index - 1; i++) {
        if (current) current = current->next;
    }
    
    if (current && current->type) {
        printf("DEBUG: Checking if parameter %d of function %s is reference\n", param_index, func_name ? func_name : "null");
        return (strstr(current->type, "&") != NULL);
    }
    
    return 0;
}