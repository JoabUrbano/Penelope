#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashMap.h"

// Função auxiliar para copiar um Data
Data copy_data(Data src) {
    Data dest;
    dest.type = strdup(src.type);

    if (strcmp(src.type, "int") == 0) {
        dest.value.intVal = src.value.intVal;
    } else if (strcmp(src.type, "float") == 0) {
        dest.value.doubleVal = src.value.doubleVal;
    } else if (strcmp(src.type, "string") == 0) {
        dest.value.strVal = strdup(src.value.strVal);
    } else if (strcmp(src.type, "bool") == 0) {
        dest.value.intVal = src.value.intVal;
    } else {
        dest.value.strVal = NULL;
    }

    return dest;
}

// Libera a memória interna de um Data
void free_data(Data data) {
    // Check string type before freeing the type pointer
    int is_string = (data.type != NULL && strcmp(data.type, "string") == 0);
    
    if (data.type != NULL) {
        free(data.type);
    }
    
    if (is_string && data.value.strVal != NULL) {
        free(data.value.strVal);
    }
}

Node* find_node(HashMap* map, char* key) {
    Node* current = map->nodes;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void insert_node(HashMap* map, char* key, Data value) {
    Node* existingNode = find_node(map, key);
    if (existingNode != NULL) {
        // Sobrescreve valor anterior
        free_data(existingNode->value);
        existingNode->value = copy_data(value);
        return;
    }

    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->key = strdup(key);
    newNode->value = copy_data(value);
    newNode->next = NULL;

    if (map->nodes == NULL) {
        map->nodes = newNode;
    } else {
        Node* current = map->nodes;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
}

void free_node(Node* node) {
    if (node != NULL) {
        free(node->key);
        free_data(node->value);
        free(node);
    }
}

void remove_node(HashMap* map, char* key) {
    Node* current = map->nodes;
    Node* previous = NULL;

    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            if (previous == NULL) {
                map->nodes = current->next;
            } else {
                previous->next = current->next;
            }

            free_node(current);
            return;
        }
        previous = current;
        current = current->next;
    }

    printf("Chave '%s' não encontrada.\n", key);
}

void print_map(HashMap* map) {
    Node* current = map->nodes;

    printf("HashMap:\n");
    while (current != NULL) {
        printf("  %s => ", current->key);

        if (strcmp(current->value.type, "int") == 0) {
            printf("%d", current->value.value.intVal);
        } else if (strcmp(current->value.type, "float") == 0) {
            printf("%f", current->value.value.doubleVal);
        } else if (strcmp(current->value.type, "string") == 0) {
            if (current->value.value.strVal != NULL) {
                printf("%s", current->value.value.strVal);
            } else {
                printf("(string nula)");
            }
        } else if (strcmp(current->value.type, "bool") == 0) {
            printf("%d", current->value.value.intVal);
        } else if (strcmp(current->value.type, "void") == 0) {
            printf("(função void)");
        } else if (strstr(current->value.type, "[]") != NULL) {
            printf("[array do tipo %s]", current->value.type);
        } else {
            printf("(tipo desconhecido)");
        }

        printf("\n");
        current = current->next;
    }
}

void free_map(HashMap* map) {
    Node* current = map->nodes;
    Node* nextNode;

    while (current != NULL) {
        nextNode = current->next;
        free_node(current);
        current = nextNode;
    }

    map->nodes = NULL;
}

// int main() {
//     HashMap map = { NULL };

//     Data nome = { strdup("string"), .value.strVal = strdup("Luiz") };
//     Data idade = { strdup("int"), .value.intVal = 25 };
//     Data nota = { strdup("float"), .value.doubleVal = 9.5 };

//     insert_node(&map, "nome", nome);
//     insert_node(&map, "idade", idade);
//     insert_node(&map, "nota", nota);

//     print_map(&map);

//     remove_node(&map, "idade");
//     printf("\nDepois de remover 'idade':\n");
//     print_map(&map);

//     free_map(&map);
//     return 0;
// }
