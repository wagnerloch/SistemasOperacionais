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

#define NMAX 1000	// Número máximo de processos

/**Estrutura dos Processos**/
struct processos {
    int executar;   // Flag para marcar que o processo está pronto para ser executado
    int executando; // Flag para marcar se o processo está sendo executado no momento
    int chegada;    // Momento em que o SO recebe o processo, vem do arquivo de entrada
    int lancamento; // Tempo/Momento que o SO é efetivamento lançado
    int duracao;    // Tempo de Duração do processo, vem do arquivo de entrada
    int execucao;   // Tempo de Execução, Duração Observada
    int memoria;    // Quantidade de memória necessária, vem do arquivo de entrada
    int prioridade; // Prioridade do Processo, vem do arquivo de entrada
    int pronto;     // Flag para marcar se o processo está pronto
    int noSistema;  // Flag para marcar se o processo está utilizando recursos do sistema
    int ciclo;      // Contador de ciclo do processo
} processos [NMAX];

int tempo;
int slice;
int numCPU;
int CPUdisponivel;
int qtdProcessos;
int memoriaSistema;

/**Declaração das Funções**/
int lerArquivo (char *nomeArquivo);
int haProcessoNaFila ();
int haProcessoParaExecutar ();
void rodarTempo ();
void receberProcessos ();
void executaProcesso (int processo);
void gerarSaida ();

int main(int argc, char *argv[]) {
    if (argc < 5) { //Argumentos não foram passados via linha de comando
        printf("Por favor, passar argumentos via linha de comando!\n");
        return 0;
    }
    tempo = -1;
    numCPU = atoi(argv[1]);                                         //Quantidade de CPUs disponíveis
    CPUdisponivel = numCPU;
    slice = atoi(argv[2]);                                          //Slice, incremento do tempo
    memoriaSistema = atoi(argv[3]);                                 //Quantidade de memória disponível
    qtdProcessos = lerArquivo(argv[4]);                             //Pega a quantidade de processos que tem na fila de execução
    while (haProcessoNaFila()) {
        rodarTempo();
        receberProcessos();
    }    
    gerarSaida();
    printf("Tempo total para executar todos os processos: %d\n", tempo);
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
        processos[i].execucao = 0;
        processos[i].pronto = 0;
        processos[i].noSistema = 0;
        processos[i].ciclo = 0;
        i++;
    }
    fclose(arquivo);
    return i;
}

/**Checa na fila de processos se há processos que ainda não foram concluídos**/
int haProcessoNaFila () {
    int i;
    for (i = 0; i < qtdProcessos; i++) {
        if(processos[i].pronto == 0) {
            return 1;
        }
    }
    return 0;
}

/**Função que analisa se o processo pode ser executado e o manda para execução**/
void receberProcessos () {
    int i = 0;
    //Executa o for em paralelo, levando em consideração o número de CPUs
    #pragma omp parallel for schedule(static, 1) num_threads(numCPU)
    for (i = 0; i < qtdProcessos; i++) {
        if (processos[i].executar == 1) {                               //Processo pronto para ser executado
            if (CPUdisponivel > 0) {                                    //Existe CPU disponível
                if (processos[i].executando == 0) {                     //Processo não está sendo executado
                    if (processos[i].pronto == 0) {                     //Processo não está pronto
                        if (memoriaSistema > processos[i].memoria) {    //Existe memória disponível
                            if(processos[i].chegada <= tempo) {         //Processo não pode ser executado antes do seu tempo
                                CPUdisponivel--;
                                memoriaSistema -= processos[i].memoria;
                                processos[i].executando = 1;            //Processo está executando
                                processos[i].executar = 0;
                                processos[i].lancamento = tempo;
                                processos[i].noSistema = 1;
                            }
                        }
                    }
                }
            } else {    // Não há CPU disponível, verificar prioridades entre os processos
                /*for (int j = 0; j < qtdProcessos; j++) {    //Procura pelo primeiro processo que está executando
                    if (processos[j].executando == 1) {
                        if (processos[j].prioridade >= processos[i].prioridade) {   //Processo novo tem prioridade mais alta
                            processos[j].executando = 0;    //Pausa o processo atual
                            memoriaSistema += processos[j].memoria; //Devolve a memória ao Sistema
                            processos[j].noSistema = 0;
                            if (processos[i].pronto == 0) {
                                if (memoriaSistema > processos[i].memoria) {
                                    memoriaSistema -= processos[i].memoria;
                                    processos[i].executando = 1;
                                    processos[i].executar = 0;
                                    processos[i].lancamento;
                                    processos[i].noSistema;
                                }
                            }
                        }
                    }
                }*/
            }
        }
    }
}

/**Função que incrementa o tempo do sistema e verifica as condições dos processos**/
void rodarTempo () {
    int i;
    tempo += slice; // Incrementa o tempo
    for (i = 0; i < qtdProcessos; i++) {               
        if (processos[i].execucao >= processos[i].duracao) {   //O Processo ja está pronto
            processos[i].pronto = 1;
            if (processos[i].noSistema == 1) {  //Verifica se o processo ainda está consumindo recursos do sistema
                CPUdisponivel++;
                memoriaSistema += processos[i].memoria;
                processos[i].noSistema = 0;
            }            
        }
        else if (processos[i].executando == 1) {
            processos[i].execucao += slice;     //Incrementa o tempo de execução do processo
            processos[i].ciclo++;
            if (processos[i].ciclo == 10) {
                if (processos[i].prioridade > 0) {
                    if (processos[i].prioridade < 4) {
                        processos[i].prioridade++;
                    }
                }
                processos[i].ciclo = 0;
            }
        }
        if (tempo >= processos[i].chegada) {    //O Processo deve ser executado
            /*printf("CPU DISPONIVEL: %d\n", CPUdisponivel);
            if (CPUdisponivel == 0) {   //Condição crítica, verificar prioridades entre os processos
                for (int j = 0; j < qtdProcessos; j++) {
                    if (processos[j].executando == 1) { //Pega o processo que está executando há mais tempo
                        if (processos[j].prioridade >= processos[i].prioridade) {   //Pausa o processo e começa outro
                            printf("TROCA DE PROCESSO\n");
                            processos[j].executando = 0;
                            memoriaSistema += processos[j].memoria;
                            if (memoriaSistema >= processos[i].memoria) {
                                processos[i].executar = 0;
                                processos[i].executando = 1;
                                processos[i].lancamento = tempo;
                                processos[i].noSistema = 1;
                                break;
                            }
                        }
                    }
                }
            }*/
            processos[i].executar = 1;  // Comando avisando que o processo deve ser executado  
        } 
    }    
    return;
}

/**Função que gera a saída do sistema conforme requisito**/
void gerarSaida () {
    int i;
    for (i = 0; i < qtdProcessos; i++) {
        printf("%d, %d, %d, %d\n", processos[i].chegada, processos[i].lancamento, processos[i].duracao, processos[i].execucao);
    }
}
