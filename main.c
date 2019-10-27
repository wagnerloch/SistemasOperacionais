#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/**Estrutura dos Processos**/
struct processos {
    int executando; // Flag para marcar se o processo est� sendo executado no momento
    int chegada;    // Momento em que o SO recebe o processo, vem do arquivo de entrada
    int lancamento; // Tempo/Momento que o SO � efetivamento lan�ado
    int duracao;    // Tempo de Dura��o do processo, vem do arquivo de entrada
    int execucao;   // Tempo de Execu��o, Dura��o Observada
    int memoria;    // Quantidade de mem�ria necess�ria, vem do arquivo de entrada
    int prioridade; // Prioridade do Processo, vem do arquivo de entrada
    int pronto;     // Flag para marcar se o processo est� pronto
} processos [100];

int tempo;
int slice;
int numCPU;
int qtdProcessos;
int memoriaSistema;

/**Declara��o das Fun��es**/
int lerArquivo (char *nomeArquivo);
void receberProcessos ();
void executaProcesso (int processo);
void gerarSaida ();

int main(int argc, char *argv[])
{
    #pragma omp parallel
    {
        int n = omp_get_num_threads();
        int id = omp_get_thread_num();
        printf("Ol� da thred %d de %d\n", id, n);
    }
    tempo = 0;
    numCPU = atoi(argv[1]);                 //Quantidade de CPUs dispon�veis
    slice = atoi(argv[2]);                  //Slice, incremento do tempo
    memoriaSistema = atoi(argv[3]);         //Quantidade de mem�ria dispon�vel
    qtdProcessos = lerArquivo(argv[4]);     //Pega a quantidade de processos que tem na fila de execu��o
    receberProcessos();
    gerarSaida();
    return 0;
}

/**L� o arquivo com as informa��es dos processos e retorna a quantidade de processos**/
int lerArquivo (char *nomeArquivo) {
    FILE *arquivo;
    int i = 0;
    if ((arquivo = fopen(nomeArquivo, "r")) == NULL) {
        printf("Erro ao abrir arquivo '%s'!\n", nomeArquivo);
    }
    while(!feof(arquivo)) {
        fscanf(arquivo, "%d,%d,%d,%d", &processos[i].chegada, &processos[i].duracao, &processos[i].memoria, &processos[i].prioridade);
        processos[i].executando = 0;
        i++;
    }
    fclose(arquivo);
    return i;
}

void receberProcessos () {
    int i = 0;
    for (i = 0; i < qtdProcessos; i++) {
        if (numCPU > 0) {   //Existe CPU dispon�vel
            if (memoriaSistema > processos[i].memoria) {    //Existe mem�ria dispon�vel
                numCPU--;
                memoriaSistema -= processos[i].memoria;
                executaProcesso(i);
                numCPU++;
                memoriaSistema += processos[i].memoria;
            }
        }
    }
}

void executaProcesso (int processo) {
    int tempoExecucao = 0;
    processos[processo].lancamento = tempo; //Momento em que o processo � executado pelo SO
    while (tempoExecucao < processos[processo].duracao) {
        tempo += slice;
        tempoExecucao += slice;
    }
    processos[processo].execucao = tempoExecucao;
    return;
}

void gerarSaida () {
    int i;
    for (i = 0; i < qtdProcessos; i++) {
        printf("%d, %d, %d, %d\n", processos[i].chegada, processos[i].lancamento, processos[i].duracao, processos[i].execucao);
    }
}
