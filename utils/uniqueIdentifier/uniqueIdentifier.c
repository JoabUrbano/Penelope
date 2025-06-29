#include <stdio.h>
#include <stdlib.h>
#include <uuid/uuid.h>

char* uniqueIdentifier() {
    uuid_t uuid;
    uuid_generate_random(uuid); 

    char *str = malloc(37); 

    if (!str) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    uuid_unparse_lower(uuid, str);  

    return str;
}

// int main() {
//     char *id = uniqueIdentifier();
//     if (id) {
//         printf("UUIDv4: %s\n", id);
//         free(id);
//     } else {
//         fprintf(stderr, "Erro ao gerar UUID\n");
//     }

//     return 0;
// }
