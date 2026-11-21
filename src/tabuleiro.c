#include "tabuleiro.h"

#include <math.h>
#include <stddef.h>

#include "recursos.h"

/* altura das linhas acima da superfície do tabuleiro (evita z-fighting) */
#define ALTURA_LINHAS 0.01f

/* peças capturadas ficam ao lado do tabuleiro, em colunas de 5 */
#define CAPTURA_X       3.0f
#define CAPTURA_Y      -0.15f
#define CAPTURA_ESPACO  0.6f

bool tabuleiroCarregaRecursos(Tabuleiro *tab)
{
    char caminho[1024];

    if (!recursosCaminho("models/tabuleiro.obj", caminho, sizeof caminho) ||
        !modeloCarregaObj(caminho, &tab->modeloTabuleiro))
        return false;

    if (!recursosCaminho("models/peca.obj", caminho, sizeof caminho) ||
        !modeloCarregaObj(caminho, &tab->modeloPeca))
        return false;

    tab->texturaTabuleiro = recursosCarregaTextura("textures/Chess_W01.tga");
    tab->texturaCacador = tab->texturaTabuleiro;
    tab->texturaLebre = recursosCarregaTextura("textures/Chess_W03.tga");

    return tab->texturaTabuleiro != 0 && tab->texturaLebre != 0;
}

static void posicionaPeca(Peca *peca, TipoPeca tipo, int i, int j)
{
    peca->tipo = tipo;
    peca->i = i;
    peca->j = j;
    peca->capturada = false;
    peca->indiceCaptura = -1;
}

void tabuleiroReinicia(Tabuleiro *tab)
{
    int k = 0;

    /* a lebre começa no centro */
    posicionaPeca(&tab->pecas[k++], LEBRE, 2, 2);

    /* caçadores: as duas primeiras linhas e as pontas da linha do meio */
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < TABULEIRO_LADO; j++)
            posicionaPeca(&tab->pecas[k++], CACADOR, i, j);

    posicionaPeca(&tab->pecas[k++], CACADOR, 2, 0);
    posicionaPeca(&tab->pecas[k++], CACADOR, 2, 4);

    tab->capturadas = 0;
}

Ponto tabuleiroPosicaoCasa(int i, int j)
{
    Ponto p = { (GLfloat) (j - 2), 0.0f, (GLfloat) (i - 2) };
    return p;
}

Ponto tabuleiroPosicaoPeca(const Peca *peca)
{
    if (!peca->capturada)
        return tabuleiroPosicaoCasa(peca->i, peca->j);

    Ponto p = {
        CAPTURA_X + CAPTURA_ESPACO * (GLfloat) (peca->indiceCaptura / TABULEIRO_LADO),
        CAPTURA_Y,
        (GLfloat) (peca->indiceCaptura % TABULEIRO_LADO - 2)
    };
    return p;
}

Peca *tabuleiroPecaEm(Tabuleiro *tab, int i, int j)
{
    for (int k = 0; k < NUM_PECAS; k++) {
        Peca *p = &tab->pecas[k];
        if (!p->capturada && p->i == i && p->j == j)
            return p;
    }
    return NULL;
}

void tabuleiroOcupacao(const Tabuleiro *tab, Ocupacao *ocupacao)
{
    for (int i = 0; i < TABULEIRO_LADO; i++)
        for (int j = 0; j < TABULEIRO_LADO; j++)
            ocupacao->casas[i][j] = CASA_VAZIA;

    for (int k = 0; k < NUM_PECAS; k++) {
        const Peca *p = &tab->pecas[k];
        if (!p->capturada)
            ocupacao->casas[p->i][p->j] = p->tipo;
    }
}

bool tabuleiroCasaSobPonto(const Ponto *p, int *i, int *j)
{
    int ci = (int) lroundf(p->z + 2.0f);
    int cj = (int) lroundf(p->x + 2.0f);

    if (!regrasDentroDoTabuleiro(ci, cj))
        return false;

    *i = ci;
    *j = cj;
    return true;
}

void tabuleiroCaptura(Tabuleiro *tab, Peca *peca)
{
    peca->capturada = true;
    peca->indiceCaptura = tab->capturadas++;
}

static void vertice(int i, int j)
{
    Ponto p = tabuleiroPosicaoCasa(i, j);
    glVertex3f(p.x, ALTURA_LINHAS, p.z);
}

static void desenhaLinhas(void)
{
    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(1.3f);
    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_LINES);
    for (int k = 0; k < TABULEIRO_LADO; k++) {
        vertice(k, 0); vertice(k, 4);   /* horizontais */
        vertice(0, k); vertice(4, k);   /* verticais */
    }
    vertice(0, 0); vertice(4, 4);       /* diagonais principais */
    vertice(4, 0); vertice(0, 4);
    glEnd();

    /* losango ligando os pontos médios das bordas */
    glBegin(GL_LINE_LOOP);
    vertice(2, 0);
    vertice(0, 2);
    vertice(2, 4);
    vertice(4, 2);
    glEnd();

    glPopAttrib();
}

void tabuleiroDesenha(const Tabuleiro *tab)
{
    glBindTexture(GL_TEXTURE_2D, tab->texturaTabuleiro);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    modeloDesenha(&tab->modeloTabuleiro);

    desenhaLinhas();
}

void tabuleiroDesenhaPeca(const Tabuleiro *tab, const Peca *peca)
{
    GLuint textura = peca->tipo == LEBRE ? tab->texturaLebre : tab->texturaCacador;
    glBindTexture(GL_TEXTURE_2D, textura);
    modeloDesenha(&tab->modeloPeca);
}

void tabuleiroDesenhaSilhuetaPeca(const Tabuleiro *tab)
{
    modeloDesenhaSimples(&tab->modeloPeca);
}
