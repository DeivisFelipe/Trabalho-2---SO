## t2 — implementação de escalonador

Você deve implementar 2 escalonadores de processos, e comparar seus desempenhos.

Um dos escalonadores é o escalonador **circular** (_round-robin_).
Nesse escalonador, os processos prontos são colocador em uma fila.
Quando há uma troca de processo, é escolhido o primeiro da fila.
Quando o estado de um processo muda para pronto (ou quando há prempção), é colocado no final da fila.

Quando um processo é retirado da fila (foi o escolhido para executar), recebe um _quantum_, uma certa quantidade de tempo de CPU que ele tem o direito de usar. Cada vez que o SO executa, o escalonador verifica se o processe em execução não ultrapassou seu _quantum_, e realiza a preempção (coloca o processo no final da fila e escolhe outro) se for o caso.
O _quantum_ é igual para todos, e geralmente corresponde ao tempo de algumas interrupções de relógio.

O outro escalonador é _processo mais curto_, no qual é escolhido o processo que se acha que vai executar por menos tempo, entre os processos prontos. O tempo esperado de execução de um processo é calculado cada vez que ele perde o processador, seja por bloqueio ou por preempção, como a média entre o tempo esperado anterior e o tempo de CPU recebido até a perda do processador. A preempção é feita como no circular, com um quantum. Considere o tempo esperado para um processo recém criado como sendo igual ao quantum.

Você deve também computar algumas métricas do sistema. Considere como um mínimo:

- tempo total de execução do sistema # Feito
- tempo total de uso da CPU (é o tempo acima menos o tempo que a CPU ficou parada) # Feito
- número de interrupções atendidas # Feito
- para cada processo:
  - tempo de retorno (tempo entre criação e término) # Feito
  - tempo total bloqueado (tempo no estado B)
  - tempo total de CPU (tempo no estado E) # Feito
  - tempo total de espera (tempo no estado P)
  - tempo médio de retorno (média do tempo entre sair do estado B e entrar no E)
  - número de bloqueios # Feito
  - número de preempções

Você deve executar o sistema em 4 configurações diferentes (cada escalonador, com quantum grande e pequeno), para dois conjuntos de programas (que serão fornecidos), e coletar as métricas.
