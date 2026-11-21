/*
 * Tabuleiro e peças: estado, posições no espaço 3D e desenho.
 */
#ifndef CTH_TABULEIRO_H
#define CTH_TABULEIRO_H

#include <stdbool.h>

#include "modelo.h"
#include "plataforma_gl.h"
#include "regras.h"
#include "util.h"

#define NUM_CACADORES 12
#define NUM_PECAS (NUM_CACADORES + 1)

typedef struct {
    TipoPeca tipo;
    int i, j;            /* casa no tabuleiro (se não capturada) */
    bool capturada;
    int indiceCaptura;   /* posição na fila de peças capturadas */
} Peca;

typedef struct {
    Peca pecas[NUM_PECAS];   /* pecas[0] é a lebre */
    int capturadas;

    Modelo modeloTabuleiro;
    Modelo modeloPeca;
    GLuint texturaTabuleiro;
    GLuint texturaCacador;
    GLuint texturaLebre;
} Tabuleiro;

/* Carrega modelos e texturas (requer contexto OpenGL ativo). */
bool tabuleiroCarregaRecursos(Tabuleiro *tab);

/* Coloca as peças na posição inicial. */
void tabuleiroReinicia(Tabuleiro *tab);

/* Posição 3D do centro da casa (i, j). */
Ponto tabuleiroPosicaoCasa(int i, int j);

/* Posição 3D de uma peça (na casa ou na fila de capturadas). */
Ponto tabuleiroPosicaoPeca(const Peca *peca);

/* Peça (não capturada) na casa (i, j), ou NULL. */
Peca *tabuleiroPecaEm(Tabuleiro *tab, int i, int j);

/* Preenche a matriz de ocupação usada pelo módulo de regras. */
void tabuleiroOcupacao(const Tabuleiro *tab, Ocupacao *ocupacao);

/* Casa do tabuleiro sob o ponto 3D `p`. Retorna false se fora do tabuleiro. */
bool tabuleiroCasaSobPonto(const Ponto *p, int *i, int *j);

/* Retira a peça do tabuleiro e a coloca na fila de capturadas. */
void tabuleiroCaptura(Tabuleiro *tab, Peca *peca);

/* Desenha o tabuleiro (modelo + linhas). */
void tabuleiroDesenha(const Tabuleiro *tab);

/* Desenha uma peça na origem (aplique a translação antes). */
void tabuleiroDesenhaPeca(const Tabuleiro *tab, const Peca *peca);

/* Desenha a silhueta de uma peça (para sombras). */
void tabuleiroDesenhaSilhuetaPeca(const Tabuleiro *tab);

#endif
