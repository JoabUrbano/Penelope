#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "uniqueIdentifier.h"

char randomChar(int index) {
    char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    return charset[index];
}

char* uniqueIdentifier() {
    int length = 24;
    char *str = malloc(length + 1); 
    if (!str) return NULL;

    srand((unsigned int) time(NULL)); 

    for (int i = 0; i < length; i++) {
        int index = rand() % 62; 
        str[i] = randomChar(index);
    }

    str[length] = '\0'; 
    return str;
}


// int main() {
//     char *identifier = uniqueIdentifier();
//     if (identifier) {
//         printf("Unique Identifier: %s\n", identifier);
//         free(identifier); 
//     } else {
//         printf("Failed to generate unique identifier.\n");
//     }
//     return 0;
// }