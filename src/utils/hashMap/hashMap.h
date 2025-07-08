#ifndef HASHMAP_H
#define HASHMAP_H

typedef struct Data
{
    char *type;
    union
    {
        int intVal;
        double doubleVal;
        char *strVal;
    } value;
} Data;

typedef struct Node
{
    char *key;
    Data value;
    struct Node *next;
} Node;

typedef struct HashMap
{
    Node *nodes;
} HashMap;

Data copy_data(Data src);

Node *find_node(HashMap *map, char *key);

void insert_node(HashMap *map, char *key, Data value);

void remove_node(HashMap *map, char *key);

void print_map(HashMap *map);

void free_node(Node *node);

void free_data(Data data);

void free_map(HashMap *map);

#endif
