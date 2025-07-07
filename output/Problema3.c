// Código C gerado a partir de Penelope
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
int linhas1;
int colunas1;
int linhas2;
int colunas2;
printf("%s", "Digite o número de linhas da primeira matriz:\n"); scanf("%d", &linhas1);
printf("%s", "Digite o número de colunas da primeira matriz:\n"); scanf("%d", &colunas1);
printf("%s", "Digite o número de linhas da segunda matriz:\n"); scanf("%d", &linhas2);
printf("%s", "Digite o número de colunas da segunda matriz:\n"); scanf("%d", &colunas2);
if (!(0)) goto L1;
printf("%s", "ERRO: Dimensões da primeira matriz inválidas!\n"); goto L2;
L1:
if (!(0)) goto L3;
printf("%s", "ERRO: Dimensões da segunda matriz inválidas!\n"); goto L4;
L3:
int** matriz1; // 2D Array declaration
// Alocação de memória para array 2D matriz1
matriz1 = malloc(linhas1 * sizeof(int*));
for (int _i = 0; _i < linhas1; _i++) {
    matriz1[_i] = malloc(colunas1 * sizeof(int));
}
int** matriz2; // 2D Array declaration
// Alocação de memória para array 2D matriz2
matriz2 = malloc(linhas2 * sizeof(int*));
for (int _i = 0; _i < linhas2; _i++) {
    matriz2[_i] = malloc(colunas2 * sizeof(int));
}
printf("%s", "Lendo matriz 1 ("); printf("%d", linhas1); printf("%s", "x"); printf("%d", colunas1); printf("%s", ")\n"); for (
int i = 0; (i < linhas1); i++) {
for (
int j = 0; (j < colunas1); j++) {
printf("%s", "Digite o elemento ["); printf("%d", i); printf("%s", ","); printf("%d", j); printf("%s", "]:"); scanf("%d", &matriz1[i][j]);
}
}
printf("%s", "Lendo matriz 2 ("); printf("%d", linhas2); printf("%s", "x"); printf("%d", colunas2); printf("%s", ")\n"); for (
int i = 0; (i < linhas2); i++) {
for (
int j = 0; (j < colunas2); j++) {
printf("%s", "Digite o elemento ["); printf("%d", i); printf("%s", ","); printf("%d", j); printf("%s", "]:"); scanf("%d", &matriz2[i][j]);
}
}
printf("%s", "\nMatrizes Lidas: \n"); printf("%s", "Matriz A:\n"); for (
int i = 0; (i < linhas1); i++) {
for (
int j = 0; (j < colunas1); j++) {
printf("%d", matriz1[i][j]); printf("%s", " "); }
printf("%s", "\n"); }
printf("%s", "Matriz B:\n"); for (
int i = 0; (i < linhas2); i++) {
for (
int j = 0; (j < colunas2); j++) {
printf("%d", matriz2[i][j]); printf("%s", " "); }
printf("%s", "\n"); }
int podesomarLinhas = (linhas1 == linhas2);
int podesomarColunas = (colunas1 == colunas2);
int podeSomar = (podesomarLinhas && podesomarColunas);
if (!(podeSomar)) goto L5;
printf("%s", "Soma possível: as matrizes têm as mesmas dimensões\n"); int** matrizSoma; // 2D Array declaration
// Alocação de memória para array 2D matrizSoma
matrizSoma = malloc(linhas1 * sizeof(int*));
for (int _i = 0; _i < linhas1; _i++) {
    matrizSoma[_i] = malloc(colunas1 * sizeof(int));
}
for (
int i = 0; (i < linhas1); i++) {
for (
int j = 0; (j < colunas1); j++) {
matrizSoma[i][j] = (matriz1[i][j] + matriz2[i][j]);
}
}
printf("%s", "RESULTADO DA SOMA (A + B):\n"); for (
int i = 0; (i < linhas1); i++) {
for (
int j = 0; (j < colunas1); j++) {
printf("%d", matrizSoma[i][j]); printf("%s", " "); }
printf("%s", "\n"); }
goto L6;
L5:
printf("%s", "ERRO: Não é possível somar as matrizes.\n"); printf("%s", "Para somar, as matrizes devem ter as mesmas dimensões.\n"); printf("%s", "Matriz A:"); printf("%d", linhas1); printf("%s", "x"); printf("%d", colunas1); printf("%s", "\n"); printf("%s", "Matriz B:"); printf("%d", linhas2); printf("%s", "x"); printf("%d", colunas2); printf("%s", "\n"); L6:
int podeMultiplicar = (colunas1 == linhas2);
if (!(podeMultiplicar)) goto L7;
printf("%s", "Multiplicação possível: A["); printf("%d", linhas1); printf("%s", "x"); printf("%d", colunas1); printf("%s", "] x B["); printf("%d", linhas2); printf("%s", "x"); printf("%d", colunas2); printf("%s", "]\n"); int linhasResultado = linhas1;
int colunasResultado = colunas2;
printf("%s", "Matriz resultado terá dimensões:"); printf("%d", linhasResultado); printf("%s", "x"); printf("%d", colunasResultado); printf("%s", "\n"); int** matrizProduto; // 2D Array declaration
// Alocação de memória para array 2D matrizProduto
matrizProduto = malloc(linhas1 * sizeof(int*));
for (int _i = 0; _i < linhas1; _i++) {
    matrizProduto[_i] = malloc(colunas2 * sizeof(int));
}
for (
int i = 0; (i < linhas1); i++) {
for (
int j = 0; (j < colunas2); j++) {
int soma = 0;
for (
int k = 0; (k < colunas1); k++) {
soma = (soma + (matriz1[i][k] * matriz2[k][j]));
}
matrizProduto[i][j] = soma;
}
}
printf("%s", "RESULTADO DA MULTIPLICAÇÃO (A x B):\n"); for (
int i = 0; (i < linhasResultado); i++) {
for (
int j = 0; (j < colunasResultado); j++) {
printf("%d", matrizProduto[i][j]); printf("%s", " "); }
printf("%s", "\n"); }
goto L8;
L7:
printf("%s", "ERRO: Não é possível multiplicar as matrizes.\n"); printf("%s", "Para multiplicar A x B, o número de colunas de A deve ser\n"); printf("%s", "igual ao número de linhas de B.\n"); printf("%s", "Matriz A:"); printf("%d", linhas1); printf("%s", "x"); printf("%d", colunas1); printf("%s", "(colunas:"); printf("%d", colunas1); printf("%s", ")\n"); printf("%s", "Matriz B:"); printf("%d", linhas2); printf("%s", "x"); printf("%d", colunas2); printf("%s", "(linhas:"); printf("%d", linhas2); printf("%s", ")\n"); L8:
L4:
L2:
return 0;
}
// Limpeza de memória dos arrays 2D
// Note: Em um código real, você deveria liberar a memória aqui
// Exemplo para matriz1: emit_2d_array_deallocation("matriz1", "linhas1");

