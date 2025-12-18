void simular_prioridade(Processo *processos_copia, int num_proc) {
    int tempo_atual = 0;
    int processos_concluidos = 0;
    int chaveamentos = 0;
    long tempo_total_chaveamento = 0;

    Fila *fila_prontos = criar_fila();
    Processo *cpu_atual = NULL;
    int t_execucao_inicio = 0; // Para a linha do tempo

    fprintf(arquivo_saida, "\n--- Simulação: Prioridade Preemptiva ---\n");

    while (processos_concluidos < num_proc) {
        
        // 1. Chegada de Processos
        for (int i = 0; i < num_proc; i++) {
            if (processos_copia[i].t_restante > 0 && processos_copia[i].tch == tempo_atual) {
                // Prioridade: Adiciona na fila mantendo a ordem (Prio, depois ID)
                inserir_ordenado_prioridade(fila_prontos, &processos_copia[i]);
            }
        }
        
        // 2. Lógica de Preempção/Troca
        int deve_chavear = 0;
        int preempcao_por_chegada = 0;
        
        if (cpu_atual != NULL) {
            if (cpu_atual->t_restante == 0) {
                // Processo terminou
                cpu_atual->t_fim_exec = tempo_atual;
                processos_concluidos++;
                deve_chavear = 1;
            } else if (fila_prontos->inicio != NULL) {
                // Verifica preempção por prioridade
                Processo *proximo = fila_prontos->inicio->processo;
                // Preempção: Prioridade do próximo (menor valor) é melhor que a atual
                if (proximo->prio < cpu_atual->prio) { 
                    deve_chavear = 1;
                    preempcao_por_chegada = 1;
                }
            }
        } else if (fila_prontos->tamanho > 0) {
            // CPU ociosa, novo processo pronto
            deve_chavear = 1; 
        }

        if (deve_chavear) {
            // Finaliza o registro do processo anterior
            if (cpu_atual != NULL) {
                registrar_timeline(t_execucao_inicio, tempo_atual, cpu_atual->id);
            }
            
            // Tratamento do processo que sai
            if (cpu_atual != NULL && cpu_atual->t_restante > 0 && preempcao_por_chegada) {
                // Coloca o processo (preemptado) de volta na fila (já está ordenado)
                // Não precisa reinserir, pois ele já estava na fila (se foi preemptado por chegada)
            }
            
            // Se houver próximo processo, faz o Chaveamento
            if (fila_prontos->tamanho > 0) {
                if (cpu_atual != NULL) { // Chaveamento real
                    chaveamentos++;
                    tempo_total_chaveamento += tTroca;
                    registrar_timeline(tempo_atual, tempo_atual + tTroca, -1);
                    tempo_atual += tTroca;
                }
                
                // Escalonamento do próximo (melhor prioridade está no topo)
                cpu_atual = desenfileirar(fila_prontos);
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
            tempo_atual++;
        } else if (fila_prontos->tamanho == 0 && processos_concluidos < num_proc) {
            // CPU Ociosa (sem processos prontos, mas com chegadas futuras)
            int ha_chegadas_futuras = 0;
            for(int i = 0; i < num_proc; i++) {
                if(processos_copia[i].t_restante > 0 && processos_copia[i].tch > tempo_atual) {
                    ha_chegadas_futuras = 1;
                    break;
                }
            }
            if(ha_chegadas_futuras) tempo_atual++; 
            else break;
        }
    }
    
    // Fim da simulação: cálculo das métricas
    if (cpu_atual != NULL && cpu_atual->t_restante == 0) {
         // O último processo terminou antes do loop, precisa registrar a fatia final
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

    fprintf(arquivo_saida, "\n--- Resultados Prioridade ---\n");
    fprintf(arquivo_saida, "Tempo médio de retorno: %.2f\n", tempo_medio_retorno);
    fprintf(arquivo_saida, "Número de chaveamentos: %d\n", chaveamentos);
    fprintf(arquivo_saida, "Overhead de chaveamento: %.3f (Tempo Total Gasto: %ld)\n", overhead, tempo_total_chaveamento);
    fprintf(arquivo_saida, "Tempo total para executar: %d\n", tempo_atual);
    fprintf(arquivo_saida, "Linha do tempo de ocupação da CPU:\n");
    // (A timeline é registrada dentro do loop)
    
    liberar_fila(fila_prontos);
}
