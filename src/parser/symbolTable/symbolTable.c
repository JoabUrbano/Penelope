#include "symbolTable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Variáveis globais para gerenciamento de escopo
char* scopeStack[MAX_SCOPE_DEPTH];
int scopeTop = -1;
HashMap symbolTable = { NULL };
char *currentScope = NULL;

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
    
    char* elementType = malloc(strlen(arrayType));
    strcpy(elementType, arrayType);
    
    // Remove o "[]" do final
    char* bracket = strstr(elementType, "[]");
    if (bracket) {
        *bracket = '\0';
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
