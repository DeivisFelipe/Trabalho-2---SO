#include "processos.h"
#include "cpu_estado.h"
#include "mem.h"
#include "es.h"
#include "tela.h"
#include <stdlib.h>
#include <stdbool.h>

// Estrutura base de um processo
struct processo_t {
    int id; // ID 0 é para o processo principal do sistema
    estado_t estado;
    acesso_t tipo_bloqueio;
    int terminal_bloqueio;
    cpu_estado_t *cpue;
    processo_t *proximo;
    int inicio_memoria;
    int fim_memoria;
    // Estatisticas
    int tempo_em_execucao;
    int tempo_em_bloqueio;
    int numero_bloqueios;
    int numero_desbloqueios;
};

// Função que cria o primeiro processo da lista de processos
processo_t* processos_cria(int id, estado_t estado , mem_t *mem, int inicio_memoria, int fim_memoria, cpu_estado_t *cpu){
  processo_t *self = malloc(sizeof(*self));
  self->id = id;
  self->estado = estado;
  self->cpue = cpue_cria();
  self->proximo = NULL;
  self->inicio_memoria = inicio_memoria;
  self->fim_memoria = fim_memoria;
  self->terminal_bloqueio = -1;
  self->tipo_bloqueio = leitura;
  self->tempo_em_execucao = 0;
  self->tempo_em_bloqueio = 0;
  self->numero_bloqueios = 0;
  self->numero_desbloqueios = 0;

  cpue_copia(cpu, self->cpue);
  //mem_printa(mem, inicio_memoria, fim_memoria);
  return self;
}

// Função que insere um novo processo a lista de processos
processo_t *processos_insere(processo_t *lista, int id, estado_t estado, int inicio_memoria, int fim_memoria, cpu_estado_t *cpu){
  processo_t *novo = malloc(sizeof(*novo));
  novo->id = id;
  novo->estado = estado;
  novo->cpue = cpue_cria();
  novo->proximo = NULL;
  novo->inicio_memoria = inicio_memoria;
  novo->fim_memoria = fim_memoria;
  novo->terminal_bloqueio = -1;
  novo->tipo_bloqueio = leitura;
  novo->tempo_em_execucao = 0;
  novo->tempo_em_bloqueio = 0;
  novo->numero_bloqueios = 0;
  novo->numero_desbloqueios = 0;

  cpue_copia(cpu, novo->cpue);
  if(lista->proximo == NULL){
    lista->proximo = novo;
  }else{
    processo_t *temp = lista->proximo;
    while(temp->proximo != NULL){
      temp = temp->proximo;
    }
    temp->proximo = novo;
  }
  return lista;
}

// Função que imprime a lista de processos
void processos_printa(processo_t *lista){
  processo_t *temp = lista;
  while(temp != NULL){
    t_printf("ID: %d, Estado: %d, Inicio: %d, Fim: %d, Bloqueio: %d, Tipo: %d", temp->id, temp->estado, temp->inicio_memoria, temp->fim_memoria, temp->terminal_bloqueio, temp->tipo_bloqueio);
    temp = temp->proximo;
  }
}

// Remove um processo
void processos_remove(processo_t *lista, processo_t *atual){
  processo_t *temp = lista;
  if(temp->proximo == NULL){
    free(lista);
    return;
  }
  while (temp->proximo != NULL && temp->proximo != atual) {
    temp = temp->proximo;
  }
  temp->proximo = atual->proximo;
  free(atual->cpue);
  free(atual);
}

// Atualiza os dados de um processo que já existe dentro da lista de processos pelo id
void processos_atualiza_dados(processo_t *lista, int id, estado_t estado, cpu_estado_t *cpu){
  processo_t *temp = lista;
  while (temp != NULL && temp->id != id)
  {
    temp = temp->proximo;
  }
  if(temp != NULL){
    temp->estado = estado;
    cpue_copia(cpu, temp->cpue);
  }  
}

// Verifica desbloqueio de processo
void processos_desbloqueia(processo_t *lista, es_t *estrada_saida){
  processo_t *temp = lista;
  while(temp != NULL){
    if(temp->estado == BLOQUEADO){
      if(es_pronto(estrada_saida, temp->terminal_bloqueio, temp->tipo_bloqueio)){
        temp->estado = PRONTO;
      }
    }
    temp = temp->proximo;
  }
}


// Pega o terminal que esta bloqueado
int processos_pega_terminal_bloqueio(processo_t *self){
  return self->terminal_bloqueio;
}

// Pega o tipo do bloqueio
int processos_pega_tipo_bloqueio(processo_t *self){
  return self->tipo_bloqueio;
}

// Add 1 numérico ao tempo de execução do processo
void processos_add_tempo_execucao(processo_t *self){
  self->tempo_em_execucao++;
}

// Add 1 numérico ao tempo de bloqueio do processo
void processos_add_tempo_bloqueio(processo_t *self){
  self->tempo_em_bloqueio++;
}

// Atualiza os dados de um processo que já existe dentro da lista de processos
void processos_atualiza_dados_processo(processo_t *self, estado_t estado, cpu_estado_t *cpu){
  self->estado = estado;
  cpue_copia(cpu, self->cpue);
}

// Atualiza o estado de um processo pelo id, se for para BLOQUEADO armazena o tipo de bloqueio e o terminal referência
void processos_atualiza_estado(processo_t *lista, int id, estado_t estado, acesso_t tipo_bloqueio, int terminal_bloqueio){
  processo_t *temp = lista;
  while (temp != NULL && temp->id != id)
  {
    temp = temp->proximo;
  }
  if(temp != NULL){
    temp->estado = estado;
    if(estado == BLOQUEADO){
      temp->tipo_bloqueio = tipo_bloqueio; 
      temp->terminal_bloqueio = terminal_bloqueio; 
    }
  }  
}


// Pega o numero de bloqueios do processo
int processos_pega_numero_bloqueios(processo_t *self){
  return self->numero_bloqueios;
}

// Pega o numero de desbloqueios do processo
int processos_pega_numero_desbloqueios(processo_t *self){
  return self->numero_desbloqueios;
}

// Pega o tempo de execucão do processo
int processos_pega_tempo_em_execucao(processo_t *self){
  return self->tempo_em_execucao;
}

// Pega o tempo de execucão de bloqueio do processo
int processos_pega_tempo_em_bloqueio(processo_t *self){
  return self->tempo_em_bloqueio;
}

// Atualiza o estado de um processo, se for para BLOQUEADO armazena o tipo de bloqueio e o terminal referência
void processos_atualiza_estado_processo(processo_t *lista, estado_t estado, acesso_t tipo_bloqueio, int terminal_bloqueio){
  processo_t *temp = lista;
  temp->estado = estado; 
  if(estado == BLOQUEADO){
    temp->tipo_bloqueio = tipo_bloqueio; 
    temp->terminal_bloqueio = terminal_bloqueio; 
  }
}

// Verifica se existe um processo pelo id
bool processos_existe(processo_t *lista, int id){
  processo_t *temp = lista;
  while (temp != NULL && temp->id != id)
  {
    temp = temp->proximo;
  }
  if(temp != NULL){
    return true;
  }else{
    return false;
  }
}

// Pega o processo pelo id
processo_t *processos_pega_processo(processo_t *lista, int id){
  processo_t *temp = lista;
  while (temp != NULL && temp->id != id)
  {
    temp = temp->proximo;
  }
  if(temp != NULL){
    return temp;
  }
  return NULL;
}

// Pega o primeiro processo que estiver pronto sem contar o SO
processo_t *processos_pega_pronto(processo_t *lista){
  processo_t *temp = lista->proximo;
  while (temp != NULL && temp->estado != PRONTO) {
    temp = temp->proximo;
  }
  if(temp != NULL){
    return temp;
  }
  return NULL;
}

// Pega o processo que estiver em execução
processo_t *processos_pega_execucao(processo_t *lista){
  processo_t *temp = lista;
  while (temp != NULL && temp->estado != EXECUCAO)
  {
    temp = temp->proximo;
  }
  if(temp != NULL){
    return temp;
  }
  return NULL;
}

// Pega a cpu de um processo
cpu_estado_t *processos_pega_cpue(processo_t *self){
  return self->cpue;
}

// Pega o id de um processo
int processos_pega_id(processo_t *self){
  return self->id;
}

// Pega o estado de um processo
estado_t processos_pega_estado(processo_t *self){
  return self->estado;
}

// Pega fim memoria processo
int processos_pega_fim(processo_t *self){
  return self->fim_memoria;
}

// Pega inicio memoria processo
int processos_pega_inicio(processo_t *self){
  return self->inicio_memoria;
}

// Destroi todos os processos
void processos_destroi(processo_t *lista){
  processo_t *temp = lista;
  while (temp == NULL)
  {
    processo_t *aux = temp->proximo;
    free(temp);
    temp = aux;
  }
  free(lista);
  return;
}

