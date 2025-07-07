#include "expressionUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Gera código C para uma expressão
char* expression_to_c_code(ExpressionResult* expr) {
    static char buffer[256];
    
    if (!expr) {
        strcpy(buffer, "0");
        return buffer;
    }
    
    if (strcmp(expr->type, "int") == 0) {
        snprintf(buffer, sizeof(buffer), "%d", expr->intVal);
    } else if (strcmp(expr->type, "float") == 0) {
        snprintf(buffer, sizeof(buffer), "%f", expr->doubleVal);
    } else if (strcmp(expr->type, "bool") == 0) {
        snprintf(buffer, sizeof(buffer), "%d", expr->intVal);
    } else if (strcmp(expr->type, "string") == 0) {
        snprintf(buffer, sizeof(buffer), "\"%s\"", expr->strVal ? expr->strVal : "");
    } else {
        strcpy(buffer, "0");
    }
    
    return buffer;
}

// Função para imprimir valores durante análise semântica (comentada para geração de código C)
void print_value(ExpressionResult* expr) {
    // Não gera saída em tempo de execução durante análise - apenas gera código C
    /*
    if (!expr) return;
    
    if (strcmp(expr->type, "int") == 0) {
        printf("%d ", expr->intVal);
    } else if (strcmp(expr->type, "float") == 0) {
        printf("%.6g ", expr->doubleVal);
    } else if (strcmp(expr->type, "bool") == 0) {
        printf("%s ", expr->intVal ? "true" : "false");
    } else if (strcmp(expr->type, "string") == 0) {
        // Trata literais de string - remove aspas se presentes
        if (expr->strVal && expr->strVal[0] == '"' && expr->strVal[strlen(expr->strVal)-1] == '"') {
            // Cria uma string temporária sem aspas
            char* temp = strdup(expr->strVal);
            temp[strlen(temp)-1] = '\0';  // Remove aspas finais
            printf("%s ", temp + 1);      // Pula aspas iniciais
            free(temp);
        } else {
            printf("%s ", expr->strVal ? expr->strVal : "");
        }
    } else {
        printf("unknown ");
    }
    */
}

// Função para avaliar números a partir de strings
double evaluate_number(char *str) {
    return atof(str);
}
