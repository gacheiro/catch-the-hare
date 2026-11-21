/*
 * Testes das regras do jogo (não dependem de OpenGL).
 */
#include <stdio.h>
#include <string.h>

#include "regras.h"

static int falhas = 0;
static int total = 0;

#define VERIFICA(cond)                                                    \
    do {                                                                  \
        total++;                                                          \
        if (!(cond)) {                                                    \
            falhas++;                                                     \
            fprintf(stderr, "%s:%d: falhou: %s\n", __FILE__, __LINE__, #cond); \
        }                                                                 \
    } while (0)

static Ocupacao ocupacao;
static TipoPeca (*const tab)[TABULEIRO_LADO] = ocupacao.casas;

static void limpa(void)
{
    memset(&ocupacao, 0, sizeof ocupacao);
}

static bool valida(TipoPeca jogador, int i0, int j0, int i1, int j1)
{
    return regrasValidaJogada(&ocupacao, jogador, i0, j0, i1, j1).valida;
}

static void testeCaminhos(void)
{
    /* casa par (centro) tem diagonais */
    VERIFICA(regrasExisteCaminho(2, 2, 1, 1));
    VERIFICA(regrasExisteCaminho(2, 2, 3, 1));
    /* casa ímpar não tem */
    VERIFICA(!regrasExisteCaminho(1, 2, 0, 1));
    VERIFICA(regrasExisteCaminho(1, 2, 0, 2));
    VERIFICA(regrasExisteCaminho(1, 2, 1, 3));
}

static void testeMovimentoSimples(void)
{
    limpa();
    tab[2][2] = LEBRE;

    VERIFICA(valida(LEBRE, 2, 2, 1, 2));
    VERIFICA(valida(LEBRE, 2, 2, 1, 1));   /* diagonal a partir de casa par */
    VERIFICA(!valida(LEBRE, 2, 2, 2, 2));  /* não sair do lugar */
    VERIFICA(!valida(LEBRE, 2, 2, 0, 0));  /* duas casas sem captura */
    VERIFICA(!valida(LEBRE, 2, 2, 0, 1));  /* movimento de cavalo */
    VERIFICA(!valida(CACADOR, 2, 2, 1, 2)); /* peça do adversário */

    limpa();
    tab[1][2] = LEBRE;
    VERIFICA(!valida(LEBRE, 1, 2, 0, 1));  /* diagonal a partir de casa ímpar */
}

static void testeLimites(void)
{
    limpa();
    tab[0][0] = CACADOR;
    VERIFICA(!valida(CACADOR, 0, 0, -1, 0));
    VERIFICA(!valida(CACADOR, 0, 0, 0, 5));
    VERIFICA(!regrasDentroDoTabuleiro(5, 0));
    VERIFICA(regrasDentroDoTabuleiro(4, 4));
}

static void testeCacador(void)
{
    limpa();
    tab[2][2] = CACADOR;
    VERIFICA(valida(CACADOR, 2, 2, 3, 3));
    VERIFICA(!valida(CACADOR, 2, 2, 4, 4));  /* só uma casa */

    tab[3][2] = LEBRE;
    VERIFICA(!valida(CACADOR, 2, 2, 3, 2));  /* casa ocupada */
    VERIFICA(!valida(CACADOR, 2, 2, 4, 2));  /* caçador não salta */
}

static void testeCaptura(void)
{
    limpa();
    tab[2][2] = LEBRE;
    tab[1][1] = CACADOR;
    tab[2][3] = CACADOR;

    Jogada j = regrasValidaJogada(&ocupacao, LEBRE, 2, 2, 0, 0);
    VERIFICA(j.valida && j.captura && j.ci == 1 && j.cj == 1);

    j = regrasValidaJogada(&ocupacao, LEBRE, 2, 2, 2, 4);
    VERIFICA(j.valida && j.captura && j.ci == 2 && j.cj == 3);

    /* destino ocupado */
    tab[0][0] = CACADOR;
    VERIFICA(!valida(LEBRE, 2, 2, 0, 0));

    /* salto sobre casa vazia */
    VERIFICA(!valida(LEBRE, 2, 2, 4, 4));

    /* salto de 3 casas */
    limpa();
    tab[0][0] = LEBRE;
    tab[0][1] = CACADOR;
    VERIFICA(!valida(LEBRE, 0, 0, 0, 3));

    /* salto diagonal a partir de casa ímpar não existe */
    limpa();
    tab[1][0] = LEBRE;
    tab[2][1] = CACADOR;
    VERIFICA(!valida(LEBRE, 1, 0, 3, 2));
}

static void testeProximoJogador(void)
{
    VERIFICA(regrasProximoJogador(LEBRE) == CACADOR);
    VERIFICA(regrasProximoJogador(CACADOR) == LEBRE);
}

int main(void)
{
    testeCaminhos();
    testeMovimentoSimples();
    testeLimites();
    testeCacador();
    testeCaptura();
    testeProximoJogador();

    printf("%d/%d verificacoes passaram\n", total - falhas, total);
    return falhas == 0 ? 0 : 1;
}
