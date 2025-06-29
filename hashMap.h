#ifndef HASHMAP_H
#define HASHMAP_H

typedef struct Node {
    char* key;
    char* value;
    struct Node* next;  
} Node;

typedef struct HashMap {
    Node* nodes;
} HashMap;

Node* find_node(HashMap* map, char* key);

void insert_node(HashMap* map, char* key, char* value);

void remove_node(HashMap* map, char* key);

void print_map(HashMap* map);

void free_node(Node* node);

void free_map(HashMap* map);

#endif 