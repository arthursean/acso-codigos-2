import csv
import sys
from dataclasses import dataclass, field
from typing import List, Dict

@dataclass
class Processo:
    id: int
    t_chegada: int
    prioridade: int
    t_cpu_total: int
    t_restante: int = 0
    t_conclusao: int = 0
    t_inicio: int = -1

    def __post_init__(self):
        self.t_restante = self.t_cpu_total

class SimuladorEscalonamento:
    def __init__(self, n_proc, quantum, t_troca, processos_lista):
        self.n_proc = n_proc
        self.quantum = quantum
        self.t_troca = t_troca
        self.processos_originais = processos_lista

    def reset_processos(self) -> List[Processo]:
        return [Processo(p.id, p.t_chegada, p.prioridade, p.t_cpu_total) for p in self.processos_originais]

    def rodar_round_robin(self):
        processos = self.reset_processos()
        tempo_atual = 0
        fila = []
        timeline = []
        processos_finalizados = []
        processos_restantes = sorted(processos, key=lambda p: (p.t_chegada, p.id))
        
        ultimo_id = -1
        num_trocas = 0
        total_tempo_troca = 0

        while len(processos_finalizados) < self.n_proc:
            # Adicionar processos que chegaram à fila
            while processos_restantes and processos_restantes[0].t_chegada <= tempo_atual:
                fila.append(processos_restantes.pop(0))

            if not fila:
                timeline.append("Ocioso")
                tempo_atual += 1
                continue

            p = fila.pop(0)

            # Verificar se houve troca de contexto
            if ultimo_id != -1 and ultimo_id != p.id:
                for _ in range(self.t_troca):
                    timeline.append("Escalonador")
                    tempo_atual += 1
                    # Adicionar quem chegar durante a troca
                    while processos_restantes and processos_restantes[0].t_chegada <= tempo_atual:
                        fila.append(processos_restantes.pop(0))
                num_trocas += 1
                total_tempo_troca += self.t_troca

            # Execução
            duracao = min(p.t_restante, self.quantum)
            for _ in range(duracao):
                timeline.append(f"P{p.id}")
                tempo_atual += 1
                p.t_restante -= 1
                # Adicionar quem chegar durante a execução
                while processos_restantes and processos_restantes[0].t_chegada <= tempo_atual:
                    fila.append(processos_restantes.pop(0))
            
            ultimo_id = p.id

            if p.t_restante > 0:
                fila.append(p)
            else:
                p.t_conclusao = tempo_atual
                processos_finalizados.append(p)

        return self.calcular_metricas("Round Robin", processos_finalizados, timeline, num_trocas, total_tempo_troca)

    def rodar_prioridade_preemptiva(self):
        processos = self.reset_processos()
        tempo_atual = 0
        timeline = []
        processos_finalizados = []
        processos_ativos = []
        processos_restantes = sorted(processos, key=lambda p: (p.t_chegada, p.id))
        
        atual = None
        num_trocas = 0
        total_tempo_troca = 0

        while len(processos_finalizados) < self.n_proc:
            # Adicionar recém-chegados
            while processos_restantes and processos_restantes[0].t_chegada <= tempo_atual:
                processos_ativos.append(processos_restantes.pop(0))

            if not processos_ativos:
                timeline.append("Ocioso")
                tempo_atual += 1
                continue

            # Escolher processo de maior prioridade (menor valor numérico)
            # Desempate pelo ID (menor ID primeiro)
            processos_ativos.sort(key=lambda p: (p.prioridade, p.id))
            escolhido = processos_ativos[0]

            # Troca de contexto
            if atual is not None and escolhido.id != atual.id:
                for _ in range(self.t_troca):
                    timeline.append("Escalonador")
                    tempo_atual += 1
                    while processos_restantes and processos_restantes[0].t_chegada <= tempo_atual:
                        processos_ativos.append(processos_restantes.pop(0))
                num_trocas += 1
                total_tempo_troca += self.t_troca
                # Re-selecionar após troca (alguém mais prioritário pode ter chegado)
                processos_ativos.sort(key=lambda p: (p.prioridade, p.id))
                escolhido = processos_ativos[0]

            atual = escolhido
            timeline.append(f"P{atual.id}")
            tempo_atual += 1
            atual.t_restante -= 1

            if atual.t_restante == 0:
                atual.t_conclusao = tempo_atual
                processos_finalizados.append(atual)
                processos_ativos.remove(atual)
                # Não resetamos 'atual' para None imediatamente para evitar troca desnecessária se o próximo for o mesmo ID
                # Mas como ele terminou, o próximo ciclo naturalmente tratará a troca

        return self.calcular_metricas("Prioridade Preemptiva", processos_finalizados, timeline, num_trocas, total_tempo_troca)

    def calcular_metricas(self, nome, finalizados, timeline, trocas, tempo_troca):
        # Tempo de Retorno (Turnaround) = Tempo de Conclusão - Tempo de Chegada
        turnarounds = [p.t_conclusao - p.t_chegada for p in finalizados]
        avg_turnaround = sum(turnarounds) / self.n_proc
        total_simulacao = len(timeline)
        overhead = (tempo_troca / total_simulacao) if total_simulacao > 0 else 0

        return {
            "algoritmo": nome,
            "avg_turnaround": avg_turnaround,
            "num_trocas": trocas,
            "overhead": overhead,
            "tempo_total": total_simulacao,
            "timeline": timeline,
            "processos": sorted(finalizados, key=lambda p: p.id)
        }

def carregar_arquivo(caminho):
    with open(caminho, 'r') as f:
        linhas = [l.strip() for l in f.readlines() if l.strip() and not any(c.isalpha() for c in l.split(',')[0])]
        
        # Primeira linha: nProc, quantum, tTroca
        n_proc, quantum, t_troca = map(int, linhas[0].split(','))
        
        processos = []
        for i in range(1, n_proc + 1):
            pid, tch, prio, tcpu = map(int, linhas[i].split(','))
            processos.append(Processo(pid, tch, prio, tcpu))
            
    return n_proc, quantum, t_troca, processos

def salvar_relatorio(resultados, caminho_saida):
    with open(caminho_saida, 'w', encoding='utf-8') as f:
        for res in resultados:
            f.write(f"--- ALGORITMO: {res['algoritmo']} ---\n")
            f.write(f"Tempo médio de retorno: {res['avg_turnaround']:.2f} ms\n")
            f.write(f"Número de chaveamento de processos: {res['num_trocas']}\n")
            f.write(f"Overhead de chaveamento: {res['overhead']:.4%}\n")
            f.write(f"Tempo total de execução: {res['tempo_total']} ms\n")
            f.write("\nTempos de Retorno Individuais:\n")
            for p in res['processos']:
                f.write(f"ID {p.id}: {p.t_conclusao - p.t_chegada} ms\n")
            
            f.write("\nLinha do Tempo de Ocupação da CPU:\n")
            f.write(" | ".join(res['timeline']))
            f.write("\n\n" + "="*50 + "\n\n")

if __name__ == "__main__":
    try:
        entrada = sys.argv[1]
        n, q, t, procs = carregar_arquivo(entrada)
        
        sim = SimuladorEscalonamento(n, q, t, procs)
        
        res_rr = sim.rodar_round_robin()
        res_prio = sim.rodar_prioridade_preemptiva()
        
        salvar_relatorio([res_rr, res_prio], "Saida_Resultados.txt")
        print("Simulação concluída com sucesso! Verifique 'Saida_Resultados.txt'.")
    except FileNotFoundError:
        print(f"Erro: Ficheiro '{entrada}' não encontrado.")
    except Exception as e:
        print(f"Ocorreu um erro: {e}")
