#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    char* key;
    char* value;
} Node;

typedef struct HashMap {
    Node* nodes;
    int size;
} HashMap;

void insert(HashMap* map, char* key, char* value) {
    Node* newNode = (Node*) malloc(sizeof(Node));
    newNode->key = strdup(key);     // copia a string para evitar problemas de ponteiro
    newNode->value = strdup(value);

    map->nodes = (Node*) realloc(map->nodes, sizeof(Node) * (map->size + 1));
    map->nodes[map->size] = *newNode;
    map->size++;

    free(newNode); // liberamos o wrapper, os dados já estão copiados no array
}

void remove_node(HashMap* map, char* key) {
    int index = -1;

    for (int i = 0; i < map->size; i++) {
        if (strcmp(map->nodes[i].key, key) == 0) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        printf("Chave '%s' não encontrada.\n", key);
        return;
    }

    // libera memória da chave e valor
    free(map->nodes[index].key);
    free(map->nodes[index].value);

    // desloca os elementos seguintes para preencher o "buraco"
    for (int i = index; i < map->size - 1; i++) {
        map->nodes[i] = map->nodes[i + 1];
    }

    map->size--;
    map->nodes = (Node*) realloc(map->nodes, sizeof(Node) * map->size);
}

void print_map(HashMap* map) {
    printf("HashMap (%d items):\n", map->size);
    for (int i = 0; i < map->size; i++) {
        printf("  %s => %s\n", map->nodes[i].key, map->nodes[i].value);
    }
}

void free_map(HashMap* map) {
    for (int i = 0; i < map->size; i++) {
        free(map->nodes[i].key);
        free(map->nodes[i].value);
    }
    free(map->nodes);
}

int main(int argc, char **argv) {
    HashMap map = { NULL, 0 };

    insert(&map, "nome", "Luiz");
    insert(&map, "curso", "TI");
    insert(&map, "cidade", "Natal");

    print_map(&map);

    remove_node(&map, "curso");

    printf("\nApós remover 'curso':\n");
    print_map(&map);

    free_map(&map);
    return 0;
}
