#ifndef hashMap
#define hashMap

typedef struct Node {
    char* key;
    char* value;
} Node;

typedef struct HashMap {
    Node* nodes;
    int size;
} HashMap;


void insert(HashMap* map, char* key, char* value);

void remove_node(HashMap* map, char* key);

void print_map(HashMap* map);

void free_map(HashMap* map);

#endif