// Kalline_Barreto 2320623 3WC
// Lúcio_Campos 2320955 3WC 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> // para manipulação segura de ponteiros como inteiros
#include "peqcomp.h"

#define TAM_COD 1024
#define VAR_OFFSET(idx) ((signed char)(-(idx * 4))) // deslocamento de 4 bytes por variável local na pilha

// struct para armazenar saltos pendentes (instruções iflez)
typedef struct {
    int pos;    // posição no vetor de código onde o offset será escrito
    int linha;  // linha de destino da instrução
} RegistroSalto;

RegistroSalto saltos_pendentes[14]; // lista de saltos a resolver
int num_saltos_pendentes = 0;       // contador de saltos pendentes

int linha_offsets[16]; // armazena a posição de início de cada linha de código

// função de erro padrão com mensagem e número da linha
static void error(const char* msg, int linha) {

    fprintf(stderr, "ERRO: %s na linha %d.\n", msg, linha);
    exit(EXIT_FAILURE);
}

// gera o prólogo da função: push %rbp; mov %rsp, %rbp; sub rsp, 32
void monta_pilha(unsigned char codigo[], int* pos) {

    codigo[(*pos)++] = 0x55;                 // push %rbp
    codigo[(*pos)++] = 0x48;
    codigo[(*pos)++] = 0x89;
    codigo[(*pos)++] = 0xE5;                 // mov %rsp, %rbp
    codigo[(*pos)++] = 0x48;
    codigo[(*pos)++] = 0x83;
    codigo[(*pos)++] = 0xEC;
    codigo[(*pos)++] = 0x20;                 // sub $32, %rsp
}

// carrega um operando em %eax (variável local, parâmetro ou constante)
void operando_eax(unsigned char* codigo, int* pos, char tipo, int ind_valor) {

    if (tipo == 'v') { // carrega variável local em %eax
        codigo[(*pos)++] = 0x8B;
        codigo[(*pos)++] = 0x45;
        codigo[(*pos)++] = (unsigned char)(VAR_OFFSET(ind_valor));       // vX ? mov -X*4(%rbp), %eax
    }

    else if (tipo == 'p') { // move parâmetro para %eax
        switch (ind_valor) {
        case 1: codigo[(*pos)++] = 0x89; codigo[(*pos)++] = 0xF8; break; // mov %edi, %eax
        case 2: codigo[(*pos)++] = 0x89; codigo[(*pos)++] = 0xF0; break; // mov %esi, %eax
        case 3: codigo[(*pos)++] = 0x89; codigo[(*pos)++] = 0xD0; break; // mov %edx, %eax
        default: error("Parametro invalido (esperado p1-p3).", 0);
        }
    }

    else if (tipo == '$') { // carrega constante em %eax
        codigo[(*pos)++] = 0xB8;
        memcpy(&codigo[*pos], &ind_valor, 4);
        (*pos) += 4;
    }

    else {
        error("Tipo de operando desconhecido.", 0);
    }
}

// move o conteúdo de %eax para uma variável local (em memória)
void var_local_eax(unsigned char* codigo, int* pos, int ind_var) { 
    codigo[(*pos)++] = 0x89;
    codigo[(*pos)++] = 0x45;
    codigo[(*pos)++] = (unsigned char)(VAR_OFFSET(ind_var));
}

// FUNÇÃO PRINCIPAL: gera o código de máquina baseado no código fonte SBas
funcp peqcomp(FILE* f, unsigned char codigo[]) {

    int pos = 0; // posição atual no vetor de código
    monta_pilha(codigo, &pos); // insere prólogo da função

    int linha = 1;
    int c;

    while ((c = fgetc(f)) != EOF) {

        linha_offsets[linha] = pos; // registra posição de início da linha

        switch (c) {

        case 'r': { // comando ret

            char var0_tipo;
            int ind0_var;

            if (fscanf(f, "et %c%d", &var0_tipo, &ind0_var) != 2)
                error("Comando 'ret' invalido.", linha);

            operando_eax(codigo, &pos, var0_tipo, ind0_var); // carrega valor a retornar
            codigo[pos++] = 0xC9; // leave
            codigo[pos++] = 0xC3; // ret
            break;
        }

        case 'v': { // comando de variável: atribuição ou operação

            int ind0;
            char char0;

            if (fscanf(f, "%d %c", &ind0, &char0) != 2)
                error("Comando 'v' invalido", linha);

            if (char0 == ':') { // atribuição simples

                char var1_tipo;
                int ind1_var;

                if (fscanf(f, " %c%d", &var1_tipo, &ind1_var) != 2)
                    error("Comando de atribuicao (:) invalido", linha);

                operando_eax(codigo, &pos, var1_tipo, ind1_var); // carrega valor
                var_local_eax(codigo, &pos, ind0);               // armazena em variável
            }

            else if (char0 == '=') { // operação aritmética

                char var1_tipo;
                int ind1_var;
                char op;
                char var2_tipo;
                int ind2_var;

                if (fscanf(f, " %c%d %c %c%d", &var1_tipo, &ind1_var, &op, &var2_tipo, &ind2_var) != 5)
                    error("Comando de operacao aritmetica (=) invalido", linha);

                operando_eax(codigo, &pos, var1_tipo, ind1_var); // carrega operando 1 em %eax

                // carrega segundo operando em outro registrador (dependendo do tipo)
                if (var2_tipo == 'v') {
                    codigo[pos++] = 0x8B;
                    codigo[pos++] = 0x4D;
                    codigo[pos++] = (unsigned char)(VAR_OFFSET(ind2_var));
                }
                else if (var2_tipo == '$') {
                    codigo[pos++] = 0xB9;
                    memcpy(&codigo[pos], &ind2_var, 4);
                    pos += 4;
                }
                else if (var2_tipo == 'p') {
                    switch (ind2_var) {
                    case 1: codigo[pos++] = 0x89; codigo[pos++] = 0xF9; break;
                    case 2: codigo[pos++] = 0x89; codigo[pos++] = 0xF1; break;
                    case 3: codigo[pos++] = 0x89; codigo[pos++] = 0xD1; break;
                    default: error("Parametro invalido (esperado p2-p3) como segundo operando.", linha);
                    }
                }
                else {
                    error("Tipo de operando 2 desconhecido em operacao aritmetica.", linha);
                }

                // aplica a operação aritmética desejada
                switch (op) {
                case '+': codigo[pos++] = 0x01; codigo[pos++] = 0xC8; break; // add %ecx, %eax
                case '-': codigo[pos++] = 0x29; codigo[pos++] = 0xC8; break; // sub %ecx, %eax
                case '*': codigo[pos++] = 0x0F; codigo[pos++] = 0xAF; codigo[pos++] = 0xC1; break; // imul %ecx, %eax
                default: error("Operador aritmetico invalido", linha);
                }

                var_local_eax(codigo, &pos, ind0); // armazena resultado na variável
            }

            else {
                error("Caractere desconhecido apos indice de variavel ('v')", linha);
            }

            break;
        }

        case 'i': { // comando iflez (salto condicional)

            char var0_tipo;
            int ind0_var;
            int linha_alvo;

            if (fscanf(f, "flez %c%d %d", &var0_tipo, &ind0_var, &linha_alvo) != 3)
                error("Comando 'iflez' invalido", linha);

            operando_eax(codigo, &pos, var0_tipo, ind0_var); // carrega valor a testar

            codigo[pos++] = 0x83; // cmp $0, %eax
            codigo[pos++] = 0xF8;
            codigo[pos++] = 0x00;

            // registra a posição do salto a ser resolvido depois
            saltos_pendentes[num_saltos_pendentes].pos = pos + 2;
            saltos_pendentes[num_saltos_pendentes].linha = linha_alvo;
            num_saltos_pendentes++;

            codigo[pos++] = 0x0F;
            codigo[pos++] = 0x8E; // jle (salto se <= 0)

            memset(&codigo[pos], 0x90, 4); // espaço reservado (NOPs) para o offset
            pos += 4;
            break;
        }

        default:
            error("Comando desconhecido", linha);
        }

        linha++;
        fscanf(f, " \n"); // ignora espaços entre comandos
    }

    // resolve os saltos pendentes, preenchendo os offsets corretos
    for (int i = 0; i < num_saltos_pendentes; i++) {
        RegistroSalto salto_atual = saltos_pendentes[i];
        int end_pos_salto = salto_atual.pos + 4;
        int end_destino = linha_offsets[salto_atual.linha];
        int offset = end_destino - end_pos_salto;
        memcpy(&codigo[salto_atual.pos], &offset, 4); // insere o offset calculado
    }

    return (funcp)codigo; // retorna ponteiro para o código gerado
}


