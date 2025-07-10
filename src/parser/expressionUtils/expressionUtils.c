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
// Gera código C para argumento de função (considera referência)
char* expression_to_c_argument_code(ExpressionResult* expr) {
    static char buffer[256];

    if (!expr || !expr->c_code) {
        strcpy(buffer, "0");
        return buffer;
    }

    // Se tipo termina com &, é referência → precisa de &
    if (strchr(expr->type, '&')) {
        snprintf(buffer, sizeof(buffer), "&%s", expr->c_code);
    } else {
        snprintf(buffer, sizeof(buffer), "%s", expr->c_code);
    }

    return buffer;
}

// Função para avaliar números a partir de strings
double evaluate_number(char *str) {
    return atof(str);
}
