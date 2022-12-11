; Código base do para inicialização do SO e processos
SO_LE   define 1
SO_ESCR define 2
SO_FIM  define 3
SO_CRIA define 4

; dispositivos de E/S
TERMINAL    DEFINE 4

; Programa
    ; Le o numero do processo
    cargi TERMINAL
    sisop SO_LE       ; retorna A=err, X=dado
    mvxa
    DESVZ termina    ; se o programa for 0 ele termina o sistema operacional
    sisop SO_CRIA
termina
    sisop SO_FIM


