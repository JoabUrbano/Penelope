#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../utils/hashMap/hashMap.h"

// A tabela de símbolos global é externa (definida no parser)
extern HashMap symbolTable;

void verificar_declaracao(const char *ident, const char *escopo, int linha) {
    char *fullKey = malloc(strlen(escopo) + strlen(ident) + 2); // escopo#ident
    sprintf(fullKey, "%s#%s", escopo, ident);

    Node *n = find_node(&symbolTable, fullKey);
    if (!n) {
        fprintf(stderr, "Erro semântico na linha %d: variável '%s' não declarada no escopo atual.\n", linha, ident);
    }

    free(fullKey);
}
