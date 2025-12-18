int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <arquivo_entrada.txt> <arquivo_saida.txt>\n", argv[0]);
        return 1;
    }

    int total_processos = 0;
    Processo *processos_originais = ler_arquivo_entrada(argv[1], &total_processos);

    if (processos_originais == NULL || total_processos == 0) {
        printf("Não foi possível carregar os dados dos processos ou o arquivo está vazio.\n");
        return 1;
    }

    arquivo_saida = fopen(argv[2], "w");
    if (arquivo_saida == NULL) {
        perror("Erro ao abrir o arquivo de saída");
        free(processos_originais);
        return 1;
    }
    
    fprintf(arquivo_saida, "--- APS 05 - Algoritmos de Escalonamento ---\n");
    fprintf(arquivo_saida, "N. Processos: %d, Quantum (RR): %d, Tempo de Troca: %d\n", nProc, quantum, tTroca);

    // --- Simulação 1: Round Robin ---
    Processo *copia_rr = copiar_processos(processos_originais, total_processos);
    simular_round_robin(copia_rr, total_processos);
    free(copia_rr);

    // --- Simulação 2: Prioridade Preemptiva ---
    Processo *copia_prio = copiar_processos(processos_originais, total_processos);
    simular_prioridade(copia_prio, total_processos);
    free(copia_prio);

    // Limpeza
    free(processos_originais);
    fclose(arquivo_saida);

    printf("Simulação concluída. Resultados escritos em %s\n", argv[2]);
    return 0;
}
