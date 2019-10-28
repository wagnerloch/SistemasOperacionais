/**
    Escalonador de Processos
    Trabalho realizado como requisito para disciplina de Sistemas Operacionais
    Wagner Loch
    14100530
    Eng. de Computação
    Universidade Federal de Pelotas (UFPel)
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/**Estrutura dos Processos**/
struct processos {
    int executando; // Flag para marcar se o processo está sendo executado no momento
    int chegada;    // Momento em que o SO recebe o processo, vem do arquivo de entrada
    int lancamento; // Tempo/Momento que o SO é efetivamento lançado
    int duracao;    // Tempo de Duração do processo, vem do arquivo de entrada
    int execucao;   // Tempo de Execução, Duração Observada
    int memoria;    // Quantidade de memória necessária, vem do arquivo de entrada
    int prioridade; // Prioridade do Processo, vem do arquivo de entrada
    int pronto;     // Flag para marcar se o processo está pronto
} processos [100];

int tempo;
int slice;
int numCPU;
int CPUdisponivel;
int qtdProcessos;
int memoriaSistema;

/**Declaração das Funções**/
int lerArquivo (char *nomeArquivo);
int haProcesso ();
void receberProcessos ();
void executaProcesso (int processo);
void gerarSaida ();

int main(int argc, char *argv[])
{
    if (argc < 5) { //Argumentos não foram passados via linha de comando
        printf("Por favor, passar argumentos via linha de comando!\n");
        return 0;
    }
    tempo = 0;
    numCPU = atoi(argv[1]);                                         //Quantidade de CPUs disponíveis
    CPUdisponivel = numCPU;
    slice = atoi(argv[2]);                                          //Slice, incremento do tempo
    memoriaSistema = atoi(argv[3]);                                 //Quantidade de memória disponível
    qtdProcessos = lerArquivo(argv[4]);                             //Pega a quantidade de processos que tem na fila de execução
    while (haProcesso()) {
        receberProcessos();
    }    
    gerarSaida();
    return 0;
}

/**Checa na fila de processos se há processos que ainda não foram concluídos**/
int haProcesso () {
    int i;
    for (i = 0; i < qtdProcessos; i++) {
        if(processos[i].pronto == 0) {
            return 1;
        }
    }
    return 0;
}

/**Lê o arquivo com as informações dos processos e retorna a quantidade de processos**/
int lerArquivo (char *nomeArquivo) {
    FILE *arquivo;
    int i = 0;
    if ((arquivo = fopen(nomeArquivo, "r")) == NULL) {
        printf("Erro ao abrir arquivo '%s'!\n", nomeArquivo);
    }
    while(!feof(arquivo)) {
        fscanf(arquivo, "%d,%d,%d,%d", &processos[i].chegada, &processos[i].duracao, &processos[i].memoria, &processos[i].prioridade);
        processos[i].executando = 0;
        processos[i].pronto = 0;
        i++;
    }
    fclose(arquivo);
    return i;
}

/**Função que analisa se o processo pode ser executado e o mando para execução**/
void receberProcessos () {
    int i = 0;
    #pragma omp parallel for default(shared) num_threads(numCPU)
    for (i = 0; i < qtdProcessos; i++) {
        if (CPUdisponivel > 0) {                                    //Existe CPU disponível
            if (processos[i].executando == 0) {                     //Processo não está sendo executado
                if (processos[i].pronto == 0) {                     //Processo não está pronto
                    if (memoriaSistema > processos[i].memoria) {    //Existe memória disponível
                        if(processos[i].chegada <= tempo) {         //Processo não pode ser executado antes do seu tempo
                            CPUdisponivel--;
                            memoriaSistema -= processos[i].memoria;
                            executaProcesso(i);
                            CPUdisponivel++;
                            memoriaSistema += processos[i].memoria;  
                        }
                    }
                }
            }
        }
    }
}

/**Função que de fato executa o processo e incrementa o tempo de execução**/
void executaProcesso (int processo) {
    processos[processo].executando = 1;                             //Marca flag de que tal processo está sendo executado
    int tempoExecucao = 0;
    processos[processo].lancamento = tempo;                         
    while (tempoExecucao < processos[processo].duracao) {           //Momento em que o processo é executado pelo SO
        tempo += slice;
        tempoExecucao += slice;
    }
    processos[processo].execucao = tempoExecucao;
    processos[processo].pronto = 1;
    return;
}

/**Função que gera a saída do sistema conforme requisito**/
void gerarSaida () {
    int i;
    for (i = 0; i < qtdProcessos; i++) {
        printf("%d, %d, %d, %d\n", processos[i].chegada, processos[i].lancamento, processos[i].duracao, processos[i].execucao);
    }
}
