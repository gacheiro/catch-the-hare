#include "regras.h"

#include <stdlib.h>

bool regrasDentroDoTabuleiro(int i, int j)
{
    return i >= 0 && i < TABULEIRO_LADO && j >= 0 && j < TABULEIRO_LADO;
}

bool regrasExisteCaminho(int i0, int j0, int i1, int j1)
{
    /* casas "pares" têm diagonais: podem mover-se em qualquer direção */
    if ((i0 + j0) % 2 == 0)
        return true;

    /* as demais só na horizontal ou vertical */
    return i0 == i1 || j0 == j1;
}

TipoPeca regrasProximoJogador(TipoPeca jogador)
{
    return jogador == LEBRE ? CACADOR : LEBRE;
}

Jogada regrasValidaJogada(const Ocupacao *tab,
                          TipoPeca jogador, int i0, int j0, int i1, int j1)
{
    Jogada r = { false, false, -1, -1 };

    if (!regrasDentroDoTabuleiro(i0, j0) || !regrasDentroDoTabuleiro(i1, j1))
        return r;

    /* só se move a própria peça */
    if (tab->casas[i0][j0] != jogador)
        return r;

    /* precisa sair do lugar e o destino deve estar vazio */
    if ((i0 == i1 && j0 == j1) || tab->casas[i1][j1] != CASA_VAZIA)
        return r;

    if (!regrasExisteCaminho(i0, j0, i1, j1))
        return r;

    int di = abs(i1 - i0);
    int dj = abs(j1 - j0);

    /* andar uma casa: permitido para ambos */
    if (di <= 1 && dj <= 1) {
        r.valida = true;
        return r;
    }

    /* caçadores só andam uma casa */
    if (jogador != LEBRE)
        return r;

    /* salto da lebre: exatamente 2 casas em linha reta */
    if (di > 2 || dj > 2 || (i0 + i1) % 2 != 0 || (j0 + j1) % 2 != 0)
        return r;

    int mi = (i0 + i1) / 2;
    int mj = (j0 + j1) / 2;

    if (tab->casas[mi][mj] != CACADOR)
        return r;

    r.valida = true;
    r.captura = true;
    r.ci = mi;
    r.cj = mj;
    return r;
}
