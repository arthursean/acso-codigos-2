// Lê o arquivo de entrada e carrega os processos
Processo* ler_arquivo_entrada(const char *nome_arquivo, int *total_processos) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo de entrada");
        return NULL;
    }

    char linha[256];
    
    // 1. Leitura dos parâmetros (nProc, quantum, tTroca)
    if (fgets(linha, sizeof(linha), arquivo)) {
        sscanf(linha, "%d,%d,%d", &nProc, &quantum, &tTroca);
        *total_processos = nProc;
    } else {
        fclose(arquivo);
        return NULL;
    }

    Processo *processos = (Processo*)malloc(nProc * sizeof(Processo));
    int i = 0;

    // 2. Leitura dos processos
    while (i < nProc && fgets(linha, sizeof(linha), arquivo)) {
        if (sscanf(linha, "%d,%d,%d,%d", 
                   &processos[i].id, 
                   &processos[i].tch, 
                   &processos[i].prio, 
                   &processos[i].tcpu_total) == 4) {
            processos[i].t_restante = processos[i].tcpu_total;
            processos[i].t_inicio_exec = -1;
            processos[i].t_fim_exec = -1;
            i++;
        }
    }
    
    // Ignora as linhas de legenda
    while (fgets(linha, sizeof(linha), arquivo));

    fclose(arquivo);
    return processos;
}

// Cria uma cópia dos processos para uma nova simulação
Processo* copiar_processos(Processo *originais, int num_proc) {
    Processo *copia = (Processo*)malloc(num_proc * sizeof(Processo));
    for (int i = 0; i < num_proc; i++) {
        copia[i] = originais[i];
        copia[i].t_restante = originais[i].tcpu_total; // Resetar
        copia[i].t_inicio_exec = -1;
        copia[i].t_fim_exec = -1;
    }
    return copia;
}

// Registra uma entrada na Linha do Tempo
void registrar_timeline(int t_inicio, int t_fim, int id) {
    if (id == -1) {
        fprintf(arquivo_saida, "%d-%d: Escalonador\n", t_inicio, t_fim);
    } else {
        fprintf(arquivo_saida, "%d-%d: P%d\n", t_inicio, t_fim, id);
    }
}
