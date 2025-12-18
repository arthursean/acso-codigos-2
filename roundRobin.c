void simular_round_robin(Processo *processos_copia, int num_proc) {
    int tempo_atual = 0;
    int processos_concluidos = 0;
    int chaveamentos = 0;
    long tempo_total_chaveamento = 0;

    Fila *fila_prontos = criar_fila();
    Processo *cpu_atual = NULL;
    int tempo_na_cpu = 0;
    int t_execucao_inicio = 0; // Para a linha do tempo

    fprintf(arquivo_saida, "\n--- Simulação: Round Robin (Quantum=%d) ---\n", quantum);

    while (processos_concluidos < num_proc) {
        
        // 1. Chegada de Processos
        for (int i = 0; i < num_proc; i++) {
            if (processos_copia[i].t_restante > 0 && processos_copia[i].tch == tempo_atual) {
                // RR: Adiciona no fim da fila
                enfileirar(fila_prontos, &processos_copia[i]);
            }
        }

        // 2. Lógica de Preempção/Troca
        int deve_chavear = 0;

        if (cpu_atual != NULL) {
            if (cpu_atual->t_restante == 0) {
                // Processo terminou
                cpu_atual->t_fim_exec = tempo_atual;
                processos_concluidos++;
                deve_chavear = 1;
            } else if (tempo_na_cpu >= quantum) {
                // Quantum expirou
                deve_chavear = 1;
            }
        } else if (fila_prontos->tamanho > 0) {
            // CPU ociosa, novo processo pronto
            deve_chavear = 1; 
        }

        if (deve_chavear) {
            // Finaliza o registro do processo anterior
            if (cpu_atual != NULL && tempo_na_cpu > 0) {
                registrar_timeline(t_execucao_inicio, tempo_atual, cpu_atual->id);
            }
            
            // Tratamento do processo que sai
            if (cpu_atual != NULL && cpu_atual->t_restante > 0) {
                // Coloca o processo de volta na fila (se não terminou)
                enfileirar(fila_prontos, cpu_atual); 
            }
            
            // Se houver próximo processo, faz o Chaveamento
            if (fila_prontos->tamanho > 0) {
                if (cpu_atual != NULL) { // Chaveamento real, não idle->processo
                    chaveamentos++;
                    tempo_total_chaveamento += tTroca;
                    registrar_timeline(tempo_atual, tempo_atual + tTroca, -1);
                    tempo_atual += tTroca;
                }
                
                // Escalonamento do próximo
                cpu_atual = desenfileirar(fila_prontos);
                tempo_na_cpu = 0;
                t_execucao_inicio = tempo_atual;
                
                if (cpu_atual->t_inicio_exec == -1) {
                    cpu_atual->t_inicio_exec = tempo_atual;
                }
            } else {
                cpu_atual = NULL; // CPU ociosa
            }
        }

        // 3. Execução
        if (cpu_atual != NULL) {
            cpu_atual->t_restante--;
            tempo_na_cpu++;
            tempo_atual++;
        } else if (fila_prontos->tamanho == 0 && processos_concluidos < num_proc) {
            // CPU Ociosa (sem processos prontos)
            // Se houverem processos que ainda vão chegar (t_restante > 0 e t_chegada > tempo_atual), o tempo avança
            int ha_chegadas_futuras = 0;
            for(int i = 0; i < num_proc; i++) {
                if(processos_copia[i].t_restante > 0 && processos_copia[i].tch > tempo_atual) {
                    ha_chegadas_futuras = 1;
                    break;
                }
            }
            if(ha_chegadas_futuras) tempo_atual++; 
            else break; // Todos os processos chegaram e terminaram ou ainda não chegaram.
        }
    }
    
    // Fim da simulação: cálculo das métricas
    // Se o último processo terminou sem chaveamento, registrar a fatia final.
    if (cpu_atual != NULL && cpu_atual->t_restante == 0 && tempo_na_cpu > 0) {
         registrar_timeline(t_execucao_inicio, tempo_atual, cpu_atual->id);
    }
    
    float tempo_total_simulacao = tempo_atual;
    float tempo_medio_retorno = 0;
    
    for (int i = 0; i < num_proc; i++) {
        int t_retorno = processos_copia[i].t_fim_exec - processos_copia[i].tch;
        tempo_medio_retorno += t_retorno;
    }
    tempo_medio_retorno /= num_proc;

    float overhead = (float)tempo_total_chaveamento / tempo_total_simulacao;

    fprintf(arquivo_saida, "\n--- Resultados RR ---\n");
    fprintf(arquivo_saida, "Tempo médio de retorno: %.2f\n", tempo_medio_retorno);
    fprintf(arquivo_saida, "Número de chaveamentos: %d\n", chaveamentos);
    fprintf(arquivo_saida, "Overhead de chaveamento: %.3f (Tempo Total Gasto: %ld)\n", overhead, tempo_total_chaveamento);
    fprintf(arquivo_saida, "Tempo total para executar: %d\n\n", tempo_atual);
    fprintf(arquivo_saida, "Linha do tempo de ocupação da CPU:\n");
    // (A timeline é registrada dentro do loop)

    liberar_fila(fila_prontos);
}
