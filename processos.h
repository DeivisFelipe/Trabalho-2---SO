#ifndef PROCESSOS_H
#define PROCESSOS_h

#include "mem.h"
#include "es.h"
#include "cpu_estado.h"
#include <stdbool.h>

typedef enum {
  EXECUCAO,     // Processo em execucao
  PRONTO,       // Processo pronto
  BLOQUEADO,    // Processo bloqueado
} estado_t;

typedef struct processo_t processo_t;
typedef struct historico_t historico_t;

processo_t* processos_cria(int id, estado_t estado , mem_t *mem, int inicio_memoria, int fim_memoria, cpu_estado_t *cpu, int tempo_inicio);

// Destroi todos os processos
void processos_destroi(processo_t *lista);

// Verifica desbloqueio de processo
void processos_desbloqueia(processo_t *lista, es_t *estrada_saida);

// Insere um novo processo no final da fila
processo_t *processos_insere(processo_t *lista, int id, estado_t estado, int inicio_memoria, int fim_memoria, cpu_estado_t *cpu, int tempo_inicio);

// Funcao que printa todos os processos
void processos_printa(processo_t *lista);

// Remove um processo
void processos_remove(processo_t *lista, processo_t *atual);

// Muda o estado de um processo pelo id
void processos_atualiza_estado(processo_t *lista, int id, estado_t estado, acesso_t tipo_bloqueio, int terminal_bloqueio);

// Muda o estado de um processo
void processos_atualiza_estado_processo(processo_t *lista, estado_t estado, acesso_t tipo_bloqueio, int terminal_bloqueio);

// Atualiza todas as informações de um processo pelo id
void processos_atualiza_dados(processo_t *lista, int id, estado_t estado, cpu_estado_t *cpu);

// Atualiza os dados de um processo que já existe dentro da lista de processos
void processos_atualiza_dados_processo(processo_t *selt, estado_t estado, cpu_estado_t *cpu);

// Pega o tempo de retorno do processo
int processos_pega_tempo_de_retorno(processo_t *self, int tempo_final);

// Pega o processo pelo id
processo_t *processos_pega_processo(processo_t *lista, int id);

// Pega processo em execução
processo_t *processos_pega_execucao(processo_t *lista);

// Pega processo que esta pronto
processo_t *processos_pega_pronto(processo_t *lista);

// Pega o terminal que esta bloqueado
int processos_pega_terminal_bloqueio(processo_t *self);

// Pega o tipo do bloqueio
int processos_pega_tipo_bloqueio(processo_t *self);

// Verifica se existe o processo com o id
bool processos_existe(processo_t *lista, int id);

// Pega a cpu do processo
cpu_estado_t *processos_pega_cpue(processo_t *self);

// Pega o id do processo
int processos_pega_id(processo_t *self);

// Pega fim memoria processo
int processos_pega_fim(processo_t *self);

// Pega inicio memoria processo
int processos_pega_inicio(processo_t *self);

// Add 1 numérico ao tempo de execução do processo
void processos_add_tempo_execucao(processo_t *self);

// Add 1 numérico ao tempo de bloqueio do processo
void processos_add_tempo_bloqueio(processo_t *self);

// Pega o numero de bloqueios do processo
int processos_pega_numero_bloqueios(processo_t *self);

// Pega o numero de desbloqueios do processo
int processos_pega_numero_desbloqueios(processo_t *self);

// Pega o tempo de execucão do processo
int processos_pega_tempo_em_execucao(processo_t *self);

// Pega o tempo de execucão de bloqueio do processo
int processos_pega_tempo_em_bloqueio(processo_t *self);

// Pega o estado do processo
estado_t processos_pega_estado(processo_t *self);
#endif