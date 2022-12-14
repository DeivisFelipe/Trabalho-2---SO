#include "so.h"
#include "tela.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <unistd.h>

struct so_t {
  contr_t *contr;       // o controlador do hardware
  bool paniquei;        // apareceu alguma situação intratável
  cpu_estado_t *cpue;   // cópia do estado da CPU
  processo_t *processos;
  int numero_de_processos;
  int memoria_utilizada;
  int memoria_pos;
  int memoria_pos_fim;
};

// funções auxiliares
static void init_mem(so_t *self);
static void panico(so_t *self);
void escalonador(so_t *self);
static void so_termina(so_t *self);
void funcao_teste(so_t *self);

so_t *so_cria(contr_t *contr)
{
  so_t *self = malloc(sizeof(*self));
  if (self == NULL) return NULL;
  self->contr = contr;
  self->paniquei = false;
  self->cpue = cpue_cria();
  self->memoria_utilizada = 0;
  init_mem(self); // Executa antes para saber qual o tamanho do programa principal
  self->processos = processos_cria(0, EXECUCAO, contr_mem(self->contr), 0, self->memoria_utilizada, self->cpue);
  self->numero_de_processos = 1;
  self->memoria_pos = 0;
  self->memoria_pos_fim = self->memoria_utilizada;

  // Chama a função teste que será usado para fazer analise do tempo de execucao
  funcao_teste(self);
  // coloca a CPU em modo usuário
  /*
  exec_copia_estado(contr_exec(self->contr), self->cpue);
  cpue_muda_modo(self->cpue, usuario);
  exec_altera_estado(contr_exec(self->contr), self->cpue);
  */
  return self;
}

void so_destroi(so_t *self)
{
  cpue_destroi(self->cpue);
  free(self);
}

// trata chamadas de sistema

// chamada de sistema para leitura de E/S
// recebe em A a identificação do dispositivo
// retorna em X o valor lido
//            A o código de erro
static void so_trata_sisop_le(so_t *self)
{
  // faz leitura assíncrona.
  // deveria ser síncrono, verificar es_pronto() e bloquear o processo
  int disp = cpue_A(self->cpue);

  bool pronto = es_pronto(contr_es(self->contr), disp, leitura);

  if(!pronto){
    processo_t *atual = processos_pega_execucao(self->processos);
    processos_atualiza_estado_processo(atual, BLOQUEADO, leitura, disp);
    if(processos_pega_id(atual) == 0){
      cpue_muda_modo(self->cpue, zumbi);
      cpue_muda_modo(processos_pega_cpue(atual), zumbi);
      exec_altera_estado(contr_exec(self->contr), self->cpue);
      // t_printf("Modo Zumbi - Esperando número do programa no terminal");
    }else{
      // Salvando processo atual na lista de processos e bloqueia
      processos_atualiza_dados_processo(atual, BLOQUEADO, self->cpue);

      // Salva o motivo do bloqueio
      processos_atualiza_estado_processo(atual, BLOQUEADO, leitura, disp);

      // Chama o escalonador
      escalonador(self);
    }
    return;
  }
  int val;
  err_t err = es_le(contr_es(self->contr), disp, &val);
  cpue_muda_A(self->cpue, err);
  if (err == ERR_OK) {
    cpue_muda_X(self->cpue, val);
  }
  // incrementa o PC
  cpue_muda_PC(self->cpue, cpue_PC(self->cpue)+2);
  // interrupção da cpu foi atendida
  cpue_muda_erro(self->cpue, ERR_OK, 0);
  // altera o estado da CPU (deveria alterar o estado do processo)
  exec_altera_estado(contr_exec(self->contr), self->cpue);
}

// chamada de sistema para escrita de E/S
// recebe em A a identificação do dispositivo
//           X o valor a ser escrito
// retorna em A o código de erro
static void so_trata_sisop_escr(so_t *self)
{
  // Verifica se o dispositivo está ocupado
  int disp = cpue_A(self->cpue);
  bool pronto = es_pronto(contr_es(self->contr), disp, escrita);

  // se o dispositivo estiver ocupado, bloqueia o processo
  if(!pronto){
    processo_t *atual = processos_pega_execucao(self->processos);
    processos_atualiza_estado_processo(atual, BLOQUEADO, escrita, disp);
    if(processos_pega_id(atual) == 0){
      cpue_muda_modo(self->cpue, zumbi);
      cpue_muda_modo(processos_pega_cpue(atual), zumbi);
      exec_altera_estado(contr_exec(self->contr), self->cpue);
      //t_printf("Modo Zumbi - Esperando número do programa no terminal");
    }else{
      // Salvando processo atual na lista de processos e bloqueia
      processos_atualiza_dados_processo(atual, BLOQUEADO, self->cpue);

      // Salva o motivo do bloqueio
      processos_atualiza_estado_processo(atual, BLOQUEADO, escrita, disp);

      // Chama o escalonador
      escalonador(self);
    }
    return;
  }
  // faz escrita assíncrona.
  // deveria ser síncrono, verificar es_pronto() e bloquear o processo
  int val = cpue_X(self->cpue);
  err_t err = es_escreve(contr_es(self->contr), disp, val);
  
  // Dispositivo ocupado, bloquear o processo, ver se tem algum pronto para rodar ou executa o SO
  if(err == ERR_OCUP){

  }
  cpue_muda_A(self->cpue, err);
  // interrupção da cpu foi atendida
  cpue_muda_erro(self->cpue, ERR_OK, 0);
  // incrementa o PC
  cpue_muda_PC(self->cpue, cpue_PC(self->cpue)+2);
  // altera o estado da CPU (deveria alterar o estado do processo)
  exec_altera_estado(contr_exec(self->contr), self->cpue);
}

// chamada de sistema para término do processo
static void so_trata_sisop_fim(so_t *self)
{
  // pega o numero do programa
  int num_prog = cpue_A(self->cpue);
  // se o valor do programa for 0, termina o SO
  if (num_prog == 0) {
    // termina o SO
    so_termina(self);
    return;
  }
  
  processo_t *atual = processos_pega_execucao(self->processos);
  int id = processos_pega_id(atual);
  t_printf("Processo %i finalizado", id);
  processos_remove(self->processos, atual);
  self->numero_de_processos--;
  if(self->numero_de_processos == 0){
    panico(self);
  }
  escalonador(self);
}

// função de encerramento do SO
static void so_termina(so_t *self)
{
  // bota o so em paniquei
  self->paniquei = true;
  // elimita todos procresso do sistema
  processos_destroi(self->processos);  
}

// Divide a string pelo delimitador, retornando um array de strings
char** divide_string(char* a_str, const char a_delim, int *tamanhoMemoria){
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Conta quantos elementos serão extraidos. */
    while (*tmp){
        if (a_delim == *tmp){
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    *tamanhoMemoria = *tamanhoMemoria + count;

    /* Adiciona espaço para token à direita. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Adiciona espaço para encerrar string nula para que o chamador saiba onde termina a lista de strings retornadas. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result){
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token){
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

// Verifica se a string tem um numero
int isDigit(const char *str){
    int ret = 0;

    if(!str){
      return 0;
    } 
    if(!(*str)) {
      return 0;
    }

    while( isblank( *str ) ){
      str++;
    }

    while(*str){
        if(isblank(*str)){
          return ret;
        }
        if(!isdigit(*str)){
          return 0;
        }
        ret = 1;
        str++;
    }

    return ret;
}

// chamada de sistema para criação de processo
static void so_trata_sisop_cria(so_t *self) {
  processo_t *atual = processos_pega_execucao(self->processos);
  
  int numeroPrograma = cpue_A(self->cpue);
  char p[1] = "p";
  char programa[7];
  int tamanhoMemoria = 0;
  // Leitura dos comandos do programa
  snprintf (programa, 7, "%s%d.maq", p, numeroPrograma);
  
  FILE *pont_arq;
  pont_arq = fopen(programa, "r");
  if (pont_arq == NULL) {
    // mostra erro que programa não encontrado
    t_printf("Erro na abertura do arquivo, programa não encontrado!");
    escalonador(self);
    return;
  }else{
    t_printf("Processo: %d executando", numeroPrograma);
  }
  char Linha[100];
  char *result;
  int pos = 0;
  int *valores;
  valores = malloc(tamanhoMemoria * sizeof(int));
  while (!feof(pont_arq)) {
    result = fgets(Linha, 100, pont_arq); 
    if (result){
      int posicao = 14;
      int final = sizeof(Linha) - posicao;
      char parte[final];
      memcpy(parte, &Linha[posicao], final);

      char** tokens;
      tokens = divide_string(parte, ',', &tamanhoMemoria);
      valores = realloc(valores, tamanhoMemoria * sizeof(int));

      if (tokens) {
        int i;
        int numero;
        for (i = 0; *(tokens + i); i++) {
            if(strlen(*(tokens + i)) != 0 && isDigit(*(tokens + i))) {
              numero = atoi(*(tokens + i));
              numero = numero;
              valores[pos] = numero;
              pos++;
            }
            free(*(tokens + i));
        }
      }
      free(tokens);
    }
  }

  // Salva qual vai ser a pos final do processo novo
  int fim = self->memoria_utilizada + tamanhoMemoria;

  // Salva o processo atual e bloqueia ele
  processos_atualiza_dados_processo(atual, BLOQUEADO, self->cpue);
  processos_atualiza_estado_processo(atual, BLOQUEADO, leitura, -1);

  // Reseta os valores da cpu para o novo processo
  cpue_muda_A(self->cpue, 0);
  cpue_muda_X(self->cpue, 0);
  cpue_muda_PC(self->cpue, 0);
  cpue_muda_erro(self->cpue, ERR_OK, 0);
  cpue_muda_modo(self->cpue, supervisor);

  // Pega a memoria do controlador
  mem_t *mem = contr_mem(self->contr);

  // Aumenta o número de processos
  self->numero_de_processos++;

  // Verifica se o processo já existe na lista de processos, neste caso só reinicia a cpu e pega as posições da memoria
  if(!processos_existe(self->processos, numeroPrograma)){
    self->memoria_pos = self->memoria_utilizada;
    self->memoria_pos_fim = fim;
    mem_muda_inicio_executando(mem, self->memoria_pos);
    mem_muda_fim_executando(mem, self->memoria_pos_fim);

    //mem_printa(mem);
    self->processos = processos_insere(self->processos, numeroPrograma, EXECUCAO, self->memoria_utilizada, fim, self->cpue);
    
    self->memoria_utilizada += tamanhoMemoria;
    mem_muda_utilizado(mem, self->memoria_utilizada);
  }else{
    t_printf("O processo já existe, reiniciando a cpu e pegando as posições da memoria\n");
    // Pega o programa que já existe
    processo_t *processo = processos_pega_processo(self->processos, numeroPrograma);
    // printa o id do processo
    t_printf("Processo: %d\n", processos_pega_id(processo));
    // atualisa o estado dele para em EXECUCAO
    processos_atualiza_dados_processo(processo, EXECUCAO, self->cpue);
    // Posição inicial da memoria
    int inicio = processos_pega_inicio(processo);
    // Posição final da memoria
    fim = processos_pega_fim(processo);
    // printa a posição inicial e final da memoria
    t_printf("Posição inicial da memoria: %d\n", inicio);
    t_printf("Posição final da memoria: %d\n", fim);
    // Muda as posições da memoria para o SO e para a memoria
    self->memoria_pos = inicio;
    self->memoria_pos_fim = fim;
    mem_muda_inicio_executando(mem, self->memoria_pos);
    mem_muda_fim_executando(mem, self->memoria_pos_fim);
  }

  // Insere o código do novo programa na memoria
  for (int i = 0; i < self->memoria_pos_fim-self->memoria_pos; i++) {
    if (mem_escreve(mem, i, valores[i]) != ERR_OK) {
      t_printf("so.init_mem: erro de memoria, endereco %d\n", i);
      panico(self);
    }
  }

  // Libera a memoria
  free(valores);

  // Altera a cpu em execução
  exec_altera_estado(contr_exec(self->contr), self->cpue);
  fclose(pont_arq);
  //...
}

// trata uma interrupção de chamada de sistema
static void so_trata_sisop(so_t *self)
{
  // o tipo de chamada está no "complemento" do cpue
  exec_copia_estado(contr_exec(self->contr), self->cpue);
  so_chamada_t chamada = cpue_complemento(self->cpue);
  switch (chamada) {
    case SO_LE:
      so_trata_sisop_le(self);
      break;
    case SO_ESCR:
      so_trata_sisop_escr(self);
      break;
    case SO_FIM:
      so_trata_sisop_fim(self);
      break;
    case SO_CRIA:
      so_trata_sisop_cria(self);
      break;
    default:
      t_printf("so: chamada de sistema nao reconhecida %d\n", chamada);
      panico(self);

  }
}

// trata uma interrupção de tempo do relógio
static void so_trata_tic(so_t *self)
{
  // Desbloqueia os processos disponiveis primeiro
  processos_desbloqueia(self->processos, contr_es(self->contr));
  processo_t *execucao = processos_pega_execucao(self->processos);
  if(execucao == NULL){
    escalonador(self);
  }
}

// Escalonador de processos
void escalonador(so_t *self){
  
  
  // Pega o primeiro processo que estiver pronto
  processo_t *pronto = processos_pega_pronto(self->processos);

  // Verifica se encontrou um processo
  if(pronto != NULL){
    // Encontrou um processo
    // Pega o id do processo pronto
    int id = processos_pega_id(pronto);
    t_printf("Processo: %i executando\n", id);
    cpue_copia(processos_pega_cpue(pronto), self->cpue);
  }else{
    // Não encontrou nenhum processo, ou seja, chama o SO para executar
    t_printf("Processo: SO executando\n");

    pronto = self->processos; // Pega o processo do SO

    // Zerando os valores da cpu do processo do SO para ele executar tudo de novo
    cpue_muda_A(self->cpue, 0);
    cpue_muda_X(self->cpue, 0);
    cpue_muda_PC(self->cpue, 0);
    cpue_muda_modo(self->cpue, supervisor);
  }

  // Apaga o erro
  cpue_muda_erro(self->cpue, ERR_OK, 0);

  // Atualiza o estado do processo para em EXECUCAO
  processos_atualiza_estado_processo(pronto, EXECUCAO, leitura, 1);

  // Altera a cpu em execução
  exec_altera_estado(contr_exec(self->contr), self->cpue);

  mem_t *mem = contr_mem(self->contr);
  // Posição inicial da memoria
  int inicio = processos_pega_inicio(pronto);
  // Posição final da memoria
  int fim = processos_pega_fim(pronto);

  // t_printf("ini %d, fim %d", inicio, fim);
  
  // Muda as posições da memoria para o SO e para a memoria
  self->memoria_pos = inicio;
  self->memoria_pos_fim = fim;
  mem_muda_inicio_executando(mem, self->memoria_pos);
  mem_muda_fim_executando(mem, self->memoria_pos_fim);

  //mem_printa(mem);

  //t_printf("a: %d, x: %i, Pc: %d\n", cpue_A(estado),cpue_X(estado),cpue_PC(estado));
}

// houve uma interrupção do tipo err — trate-a
void so_int(so_t *self, err_t err)
{
  switch (err) {
    case ERR_SISOP:
      so_trata_sisop(self);
      break;
    case ERR_TIC:
      so_trata_tic(self);
      break;
    default:
      t_printf("SO: interrupcao nao tratada [%s]", err_nome(err));
      self->paniquei = true;
  }
}

// retorna false se o sistema deve ser desligado
bool so_ok(so_t *self)
{
  return !self->paniquei;
}

// Retorna a lista de processos
processo_t *so_pega_processos(so_t *self) {
  return self->processos;
}


// Função utilizada para fazer analises do tempo de execução dos programa
void funcao_teste(so_t * self){
  t_ins(7, 1);
  t_ins(7, 2);
  t_ins(0, 50);
  t_ins(1, 60);
}



// carrega um programa na memória
static void init_mem(so_t *self)
{
  // programa para executar na nossa CPU
  int progr[] = {
  #include "initso.maq"
  };
  int tamanho_programa = sizeof(progr)/sizeof(progr[0]);

  t_printf("Processo: SO executando\n");

  // inicializa a memória com o programa com uma memoria nova
  mem_t *mem = contr_mem(self->contr);
  mem_muda_utilizado(mem, self->memoria_utilizada);
  mem_muda_inicio_executando(mem, 0);
  mem_muda_fim_executando(mem, tamanho_programa);
  //t_printf("Memoria Utilizada: %d\n", mem_utilizado(mem));
  for (int i = 0; i < tamanho_programa; i++) {
    if (mem_escreve(mem, i, progr[i]) != ERR_OK) {
      t_printf("so.init_mem: erro de memoria, endereco %d\n", i);
      panico(self);
    }
  }
  self->memoria_utilizada += tamanho_programa;
  mem_muda_utilizado(mem, self->memoria_utilizada);
  //mem_printa(mem);
  //t_printf("Memoria Utilizada: %d\n", mem_utilizado(mem));
}
  
static void panico(so_t *self) 
{
  //t_printf("Problema irrecuperavel no SO");
  self->paniquei = true;
}
