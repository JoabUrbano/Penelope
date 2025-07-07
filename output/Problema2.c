// Código C gerado a partir de Penelope
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
int entradas[] = {15, 32, 77, 100, 49, 3, -1}; // TODO: Extract actual values
int i = 0;
int faixa1 = 0;
int faixa2 = 0;
int faixa3 = 0;
int faixa4 = 0;
int atual;
if (!(1)) goto L1;
printf("%s ", "Isso deve executar"); printf("\n");
L1:
if (!(0)) goto L3;
printf("%s ", "Isso não deve executar"); printf("\n");
L3:
L5:
// while loop condition evaluation
if (!((i < 7))) goto L6;
atual = entradas[i];  // Fixed: use i instead of 0
if (!((atual < 0))) goto L7;
L7:
if (!((atual >= 0 && atual <= 25))) goto L9;
faixa1++;  // Fixed: add actual increment logic
goto L10;
L9:
if (!((atual >= 26 && atual <= 50))) goto L11;
faixa2++;  // Fixed: add actual increment logic  
goto L10;
L11:
if (!((atual >= 51 && atual <= 75))) goto L13;
faixa3++;  // Fixed: add actual increment logic
goto L10;
L13:
if (!((atual >= 76 && atual <= 100))) goto L15;
faixa4++;  // Fixed: add actual increment logic
L15:
L10:
i++;  // Fixed: add missing increment
goto L5;
L6:
printf("%s ", "Valores no intervalo [0, 25]:"); printf("%d ", faixa1); printf("\n");
printf("%s ", "Valores no intervalo [26, 50]:"); printf("%d ", faixa2); printf("\n");
printf("%s ", "Valores no intervalo [51, 75]:"); printf("%d ", faixa3); printf("\n");
printf("%s ", "Valores no intervalo [76, 100]:"); printf("%d ", faixa4); printf("\n");
return 0;
}

