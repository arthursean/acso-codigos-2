#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Estruturas de Dados ---

// Estrutura para representar um Processo
typedef struct {
    int id;         // ID do processo
    int tch;        // Tempo de chegada
    int prio;       // Prioridade (menor valor = maior prioridade)
    int tcpu_total; // Tempo de CPU total (Burst original)
    int t_restante; // Tempo de CPU restante
    int t_inicio_exec; // Tempo da primeira execução
    int t_fim_exec; // Tempo de conclusão
} Processo;

// Estrutura para um nó da Fila de Prontos (usando lista encadeada)
typedef struct No {
    Processo *processo;
    struct No *proximo;
} NoFila;

// Estrutura para gerenciar a Fila de Prontos
typedef struct {
    NoFila *inicio;
    NoFila *fim;
    int tamanho;
} Fila;

// --- Parâmetros Globais ---
int nProc, quantum, tTroca;
FILE *arquivo_saida;

// --- Funções Auxiliares de Gerenciamento da Fila ---

// Cria uma nova fila vazia
Fila* criar_fila() {
    Fila *fila = (Fila*)malloc(sizeof(Fila));
    fila->inicio = NULL;
    fila->fim = NULL;
    fila->tamanho = 0;
    return fila;
}

// Adiciona um processo no fim da fila (usado no RR)
void enfileirar(Fila *fila, Processo *p) {
    NoFila *novo_no = (NoFila*)malloc(sizeof(NoFila));
    novo_no->processo = p;
    novo_no->proximo = NULL;

    if (fila->fim == NULL) {
        fila->inicio = novo_no;
        fila->fim = novo_no;
    } else {
        fila->fim->proximo = novo_no;
        fila->fim = novo_no;
    }
    fila->tamanho++;
}

// Remove e retorna o processo do início da fila (usado no RR)
Processo* desenfileirar(Fila *fila) {
    if (fila->inicio == NULL) {
        return NULL;
    }
    NoFila *no_removido = fila->inicio;
    Processo *p = no_removido->processo;

    fila->inicio = fila->inicio->proximo;
    if (fila->inicio == NULL) {
        fila->fim = NULL;
    }
    fila->tamanho--;
    free(no_removido);
    return p;
}

// Adiciona um processo na fila, mantendo a ordem de prioridade/ID (usado no Prioridade)
void inserir_ordenado_prioridade(Fila *fila, Processo *p) {
    NoFila *novo_no = (NoFila*)malloc(sizeof(NoFila));
    novo_no->processo = p;
    novo_no->proximo = NULL;

    if (fila->inicio == NULL) {
        fila->inicio = novo_no;
        fila->fim = novo_no;
    } else {
        NoFila *atual = fila->inicio;
        NoFila *anterior = NULL;

        // Regra: Menor Prio > Menor ID
        while (atual != NULL && 
               (atual->processo->prio < p->prio || 
               (atual->processo->prio == p->prio && atual->processo->id < p->id))) {
            anterior = atual;
            atual = atual->proximo;
        }

        if (anterior == NULL) { // Inserir no início
            novo_no->proximo = fila->inicio;
            fila->inicio = novo_no;
        } else { // Inserir no meio ou fim
            novo_no->proximo = atual;
            anterior->proximo = novo_no;
            if (atual == NULL) {
                fila->fim = novo_no;
            }
        }
    }
    fila->tamanho++;
}

// Libera a memória alocada para a fila
void liberar_fila(Fila *fila) {
    NoFila *atual = fila->inicio;
    while (atual != NULL) {
        NoFila *proximo = atual->proximo;
        // Não liberamos o processo em si, pois eles estão na lista ProcessosOriginais
        free(atual);
        atual = proximo;
    }
    free(fila);
}
