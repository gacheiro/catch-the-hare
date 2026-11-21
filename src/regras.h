/*
 * Regras do jogo "Catch the Hare" (caça à lebre).
 *
 * Este módulo não depende de OpenGL e pode ser testado isoladamente.
 *
 * O tabuleiro tem 5x5 casas. Toda casa liga-se às vizinhas na horizontal e
 * na vertical; casas com (i + j) par também se ligam às vizinhas diagonais.
 *
 *  - Caçadores andam uma casa por vez ao longo de uma linha.
 *  - A lebre anda uma casa, ou salta sobre um caçador adjacente (em linha)
 *    caindo na casa vazia logo atrás dele, capturando-o.
 */
#ifndef CTH_REGRAS_H
#define CTH_REGRAS_H

#include <stdbool.h>

#define TABULEIRO_LADO 5

typedef enum {
    CASA_VAZIA = 0,
    LEBRE = 1,
    CACADOR = 2
} TipoPeca;

/* Conteúdo de cada casa do tabuleiro. */
typedef struct {
    TipoPeca casas[TABULEIRO_LADO][TABULEIRO_LADO];
} Ocupacao;

typedef struct {
    bool valida;
    bool captura;   /* a lebre saltou sobre um caçador */
    int ci, cj;     /* casa do caçador capturado (se captura) */
} Jogada;

/* Indica se (i, j) está dentro do tabuleiro. */
bool regrasDentroDoTabuleiro(int i, int j);

/* Indica se existe uma linha do tabuleiro partindo de (i0, j0) na direção
 * de (i1, j1). */
bool regrasExisteCaminho(int i0, int j0, int i1, int j1);

/* Valida a jogada de (i0, j0) para (i1, j1) feita por `jogador`.
 * `tab` descreve o conteúdo de cada casa. */
Jogada regrasValidaJogada(const Ocupacao *tab,
                          TipoPeca jogador, int i0, int j0, int i1, int j1);

/* Retorna o adversário do jogador dado. */
TipoPeca regrasProximoJogador(TipoPeca jogador);

#endif
