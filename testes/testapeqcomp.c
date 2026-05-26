// Kalline_Barreto 2320623 3WC
// Lúcio_Campos 2320955 3WC 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "peqcomp.h"

#define TAM_COD 1024 // define o tamanho apropriado

int main(void) {

    FILE* f = fopen("teste2.txt", "r"); // abre o arquivo contendo o código fonte SBas
    if (f == NULL) {
        perror("ERRO: Nao conseguiu abrir arquivo."); // trata erro de abertura do arquivo
        exit(1);
    }

    unsigned char codigo[TAM_COD]; // declara o vetor de armazenamento do código de máquina

    funcp func = peqcomp(f, codigo); // chama a função peqcomp para compilar o código SBas em código de máquina
    if (func == NULL) {
        fprintf(stderr, "Erro: funcao compilada invalida.\n");
        exit(1);
    }

    fclose(f); // fecha o arquivo após a leitura do código fonte

    // declara os parâmetros a serem utilizados pela função gerada
    int p1 = 4;
    int p2 = 6;
    int p3 = 1;

    // chama a função compilada, passando os três parâmetros declarados
    int result = ((int (*)(int, int, int)) func)(p1, p2, p3);

    // impressão dos parâmetros e resultado da execução da função SBas
    printf("\nChamando a funcao SBas com p1 = %d, p2 = %d, p3 = %d\n", p1, p2, p3);
    printf("\nResultado da funcao SBas: %d\n", result);

    // impressão dos primeiros 40 bytes do código gerado, em formato hexadecimal
    printf("\nPrimeiros 40 bytes do codigo de maquina gerado (em hexadecimal):\n\n");

    for (int i = 0; i < 40; i++) {
        printf("Codigo[%d] = 0x%02X\n", i, codigo[i]); // imprime cada byte em formato legível
    }

    return 0; // finaliza o programa
}


