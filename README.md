# Microcompilador SBas para x86-64

Este repositório contém a implementação do Segundo Trabalho (T2) da disciplina de Software Básico (INF1018).

## 1. Proposta do Trabalho

O objetivo principal deste projeto foi desenvolver, na linguagem C, um microcompilador capaz de traduzir arquivos de texto contendo código na linguagem SBas diretamente para código de máquina executável na arquitetura x86-64.

Para alcançar este objetivo, o trabalho exigiu a aplicação prática de conceitos fundamentais de software básico e arquitetura de computadores, incluindo:

* **Geração dinâmica** de instruções de máquina (opcodes) em vetores de bytes na memória.
* **Manipulação da pilha de execução** (Stack Frame) com a criação de prólogo e epílogo para alocação de variáveis locais.
* **Mapeamento de registradores** de acordo com a convenção System V AMD64 ABI para passagem de parâmetros e execução de operações aritméticas (utilizando `%eax` e `%ecx`).
* **Implementação de controle de fluxo** através de desvios condicionais (`iflez`), exigindo o cálculo de offsets e a resolução de saltos pendentes em duas passadas (backpatching).

## 2. Implementação e Arquitetura do Sistema

A solução foi estruturada de forma modular, separando as responsabilidades do núcleo de compilação e do ambiente de orquestração e execução de testes. A arquitetura final é composta pelos seguintes módulos principais:

* **Microcompilador (`peqcomp.c`):** Atua como o núcleo tradutor do sistema. É responsável por ler o arquivo de código-fonte SBas linha por linha, interpretar os comandos e gerar as instruções nativas correspondentes. Ele gerencia o mapeamento das variáveis na pilha, traduz operações aritméticas e gerencia uma tabela de saltos pendentes para calcular e injetar os endereços corretos de destino das instruções de desvio ao final do processo.
* **Orquestrador de Testes (`testapeqcomp.c`):** Ponto de entrada do sistema. É responsável por abrir os arquivos de teste SBas, invocar o compilador e injetar o vetor de bytes resultante em um ponteiro de função. Executa o papel de carregar o código em memória, injetar os parâmetros de execução (`p1`, `p2`, `p3`) e invocar a função gerada de forma segura, imprimindo também o dump hexadecimal do código traduzido.
* **Casos de Teste (`teste1.txt` a `teste6.txt`):** Representam os programas a serem compilados. Executam lógicas variadas, desde retornos diretos de constantes até estruturas mais complexas de loops e testes condicionais (como simulações de cálculo de fatorial), servindo para validar a integridade e os limites do código gerado pelo compilador.

## 3. Instruções de Compilação e Uso

O projeto utiliza o GCC nativo em ambiente Linux/WSL para a compilação, exigindo a desativação da proteção de execução da pilha para que o vetor de bytes compilado possa rodar em memória.

### Compilação
Para compilar o projeto e gerar o executável, execute o seguinte comando na raiz do diretório (assumindo a estrutura de pastas organizada):

```bash
gcc -Wall -Wa,--execstack -I./include -o build/testapeqcomp tests/testapeqcomp.c src/peqcomp.c
