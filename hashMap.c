#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashMap.h"

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

void insert_node(HashMap* map, char* key, char* value) {
    Node* newNode = (Node*) malloc(sizeof(Node));

    newNode->key = strdup(key);
    newNode->value = strdup(value);
    newNode->next = NULL;

    if(map->nodes == NULL) {
        map->nodes = newNode;
    } else {
        Node* existingNode = find_node(map, key);

        if(existingNode != NULL) {
            existingNode->value = strdup(value);
            free_node(newNode);
            return;

        }

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
        free(node->value);
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
        printf("  %s => %s\n", current->key, current->value);
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

int main(int argc, char **argv) {
    HashMap map = { NULL };

    insert_node(&map, "nome", "Luiz");
    insert_node(&map, "nome", "Luiz GUSTAVO");
    insert_node(&map, "curso", "TI");
    insert_node(&map, "cidade", "Natal");

    print_map(&map);

    remove_node(&map, "curso");

    printf("\nApós remover 'curso':\n");
    print_map(&map);

    free_map(&map);
    return 0;
}
