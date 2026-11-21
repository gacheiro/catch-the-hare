#include "jogo.h"

#include <stdio.h>
#include <stdlib.h>

#include "plataforma_gl.h"
#include "regras.h"
#include "tabuleiro.h"
#include "util.h"

#define TITULO_JANELA "Catch the Hare"
#define LARGURA_INICIAL 1280
#define ALTURA_INICIAL 720

#define CAMPO_DE_VISAO 50.0f
#define DISTANCIA_CAMERA 7.0f
#define PASSO_ROTACAO 2.0f

#define QUADROS_POR_SALTO 24
#define MS_POR_QUADRO 16
#define ALTURA_SALTO 0.5f
#define MAX_ANIMACOES 2

/* sombras são projetadas um pouco acima do tabuleiro */
static const GLfloat PLANO_SOMBRA[4] = { 0.0f, 1.0f, 0.0f, -0.01f };

static const GLfloat MAT_AMBIENTE[]  = { 0.4f, 0.4f, 0.4f, 1.0f };
static const GLfloat MAT_DIFUSA[]    = { 0.0f, 0.0f, 0.0f, 1.0f };
static const GLfloat MAT_ESPECULAR[] = { 0.5f, 0.5f, 0.5f, 1.0f };
static const GLfloat MAT_BRILHO[]    = { 5.0f };

typedef struct {
    Peca *peca;
    Ponto origem, destino;
    int quadro;
} Animacao;

typedef struct {
    Tabuleiro tabuleiro;
    Peca *selecao;
    TipoPeca jogadorDaRodada;

    /* câmera: rotação do mundo em torno dos eixos X e Y (graus) */
    GLfloat anguloX, anguloY;
    int largura, altura;

    /* posição da luz no espaço do tabuleiro (antes da rotação) */
    GLfloat luz[4];

    /* fila de animações: a primeira está tocando, as demais aguardam */
    Animacao animacoes[MAX_ANIMACOES];
    int numeroAnimacoes;
} Jogo;

static Jogo jogo;

/* ---- animações ---------------------------------------------------------- */

static bool animando(void)
{
    return jogo.numeroAnimacoes > 0;
}

static void tickAnimacao(int valor)
{
    (void) valor;

    if (!animando())
        return;

    Animacao *a = &jogo.animacoes[0];
    if (++a->quadro >= QUADROS_POR_SALTO) {
        for (int k = 1; k < jogo.numeroAnimacoes; k++)
            jogo.animacoes[k - 1] = jogo.animacoes[k];
        jogo.numeroAnimacoes--;
    }

    if (animando())
        glutTimerFunc(MS_POR_QUADRO, tickAnimacao, 0);

    glutPostRedisplay();
}

static void enfileiraAnimacao(Peca *peca, Ponto origem, Ponto destino)
{
    if (jogo.numeroAnimacoes >= MAX_ANIMACOES)
        return;

    Animacao a = { peca, origem, destino, 0 };
    jogo.animacoes[jogo.numeroAnimacoes++] = a;

    if (jogo.numeroAnimacoes == 1)
        glutTimerFunc(MS_POR_QUADRO, tickAnimacao, 0);
}

/* Posição em que a peça deve ser desenhada agora, levando em conta as
 * animações pendentes. */
static Ponto posicaoVisivel(const Peca *peca)
{
    for (int k = 0; k < jogo.numeroAnimacoes; k++) {
        const Animacao *a = &jogo.animacoes[k];
        if (a->peca != peca)
            continue;

        if (k > 0)  /* ainda não começou: fica onde estava */
            return a->origem;

        float t = (float) a->quadro / (float) (QUADROS_POR_SALTO - 1);
        return utilPosicaoSalto(&a->origem, &a->destino, ALTURA_SALTO, t);
    }
    return tabuleiroPosicaoPeca(peca);
}

/* ---- regras da partida -------------------------------------------------- */

static void atualizaTitulo(void)
{
    glutSetWindowTitle(jogo.jogadorDaRodada == LEBRE
                       ? TITULO_JANELA " - vez da lebre"
                       : TITULO_JANELA " - vez dos cacadores");
}

static void tentaMover(int i, int j)
{
    Ocupacao ocupacao;
    tabuleiroOcupacao(&jogo.tabuleiro, &ocupacao);

    Peca *peca = jogo.selecao;
    Jogada jogada = regrasValidaJogada(&ocupacao, jogo.jogadorDaRodada,
                                       peca->i, peca->j, i, j);
    if (!jogada.valida)
        return;

    Ponto origem = tabuleiroPosicaoPeca(peca);
    peca->i = i;
    peca->j = j;
    enfileiraAnimacao(peca, origem, tabuleiroPosicaoPeca(peca));

    if (jogada.captura) {
        Peca *capturada = tabuleiroPecaEm(&jogo.tabuleiro, jogada.ci, jogada.cj);
        if (capturada != NULL) {
            Ponto antes = tabuleiroPosicaoPeca(capturada);
            tabuleiroCaptura(&jogo.tabuleiro, capturada);
            enfileiraAnimacao(capturada, antes, tabuleiroPosicaoPeca(capturada));
        }
    }

    jogo.selecao = NULL;
    jogo.jogadorDaRodada = regrasProximoJogador(jogo.jogadorDaRodada);
    atualizaTitulo();
}

/* ---- câmera ------------------------------------------------------------- */

static void aplicaProjecao(void)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(CAMPO_DE_VISAO,
                   (GLdouble) jogo.largura / (GLdouble) jogo.altura,
                   1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

static void aplicaCamera(void)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 0.0, DISTANCIA_CAMERA,
              0.0, 0.0, 0.0,
              0.0, 1.0, 0.0);
    glRotatef(jogo.anguloX, 1.0f, 0.0f, 0.0f);
    glRotatef(jogo.anguloY, 0.0f, 1.0f, 0.0f);
}

static void luzAtual(GLfloat luz[4])
{
    /* a luz acompanha a rotação horizontal da câmera */
    utilRotacionaLuzY(jogo.luz, jogo.anguloY, luz);
}

/* ---- renderização ------------------------------------------------------- */

static void desenhaPecas(void)
{
    glEnable(GL_TEXTURE_2D);
    glColor4f(0.6f, 0.6f, 0.6f, 1.0f);

    for (int k = 0; k < NUM_PECAS; k++) {
        const Peca *p = &jogo.tabuleiro.pecas[k];
        Ponto pos = posicaoVisivel(p);

        glPushMatrix();
        glTranslatef(pos.x, pos.y, pos.z);
        tabuleiroDesenhaPeca(&jogo.tabuleiro, p);
        glPopMatrix();
    }
}

static void desenhaSombras(void)
{
    GLfloat luz[4];
    luzAtual(luz);

    glPushMatrix();
    utilMatrizSombreamento(PLANO_SOMBRA, luz);
    glColor4f(0.0f, 0.0f, 0.0f, 0.2f);

    for (int k = 0; k < NUM_PECAS; k++) {
        const Peca *p = &jogo.tabuleiro.pecas[k];
        Ponto pos = posicaoVisivel(p);

        /* peças fora do tabuleiro não projetam sombra nele */
        if (p->capturada && pos.y < 0.0f)
            continue;

        glPushMatrix();
        glTranslatef(pos.x, pos.y, pos.z);
        tabuleiroDesenhaSilhuetaPeca(&jogo.tabuleiro);
        glPopMatrix();
    }
    glPopMatrix();
}

static void desenhaSelecao(void)
{
    if (jogo.selecao == NULL)
        return;

    Ponto pos = tabuleiroPosicaoPeca(jogo.selecao);

    glPushMatrix();
    glColor4f(0.6f, 0.6f, 0.6f, 0.3f);
    glTranslatef(pos.x, pos.y + 0.12f, pos.z);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glutSolidTorus(0.15, 0.125, 10, 20);
    glPopMatrix();
}

static void desenha(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    aplicaCamera();

    GLfloat luz[4];
    luzAtual(luz);
    glLightfv(GL_LIGHT0, GL_POSITION, luz);

    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
    desenhaPecas();
    tabuleiroDesenha(&jogo.tabuleiro);

    /* sombras e efeitos não precisam de iluminação nem textura */
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glShadeModel(GL_FLAT);
    desenhaSombras();
    desenhaSelecao();

    glutSwapBuffers();
}

/* ---- entrada ------------------------------------------------------------ */

static void rotaciona(GLfloat dx, GLfloat dy)
{
    jogo.anguloX += dx;
    if (jogo.anguloX > 90.0f) jogo.anguloX = 90.0f;
    if (jogo.anguloX < 0.0f)  jogo.anguloX = 0.0f;

    jogo.anguloY += dy;

    glutPostRedisplay();
}

static void teclado(unsigned char tecla, int x, int y)
{
    (void) x;
    (void) y;

    switch (tecla) {
    case 27:  /* ESC */
        exit(EXIT_SUCCESS);
    case 'w': case 'W': rotaciona(+PASSO_ROTACAO, 0.0f); break;
    case 's': case 'S': rotaciona(-PASSO_ROTACAO, 0.0f); break;
    case 'a': case 'A': rotaciona(0.0f, -PASSO_ROTACAO); break;
    case 'd': case 'D': rotaciona(0.0f, +PASSO_ROTACAO); break;
    default: break;
    }
}

static void tecladoEspecial(int tecla, int x, int y)
{
    switch (tecla) {
    case GLUT_KEY_UP:    teclado('w', x, y); break;
    case GLUT_KEY_DOWN:  teclado('s', x, y); break;
    case GLUT_KEY_LEFT:  teclado('a', x, y); break;
    case GLUT_KEY_RIGHT: teclado('d', x, y); break;
    default: break;
    }
}

/* Converte o clique na janela para um ponto 3D no mundo usando a
 * profundidade do pixel clicado. Retorna false se clicou no fundo. */
static bool pontoClicado(int x, int y, Ponto *p)
{
    GLint viewport[4];
    GLdouble modelview[16], projecao[16];
    GLfloat profundidade;
    GLdouble wx, wy, wz;

    aplicaCamera();
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projecao);

    GLint janelaY = viewport[3] - y - 1;
    glReadPixels(x, janelaY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &profundidade);
    if (profundidade >= 1.0f)
        return false;

    if (gluUnProject(x, janelaY, profundidade, modelview, projecao, viewport,
                     &wx, &wy, &wz) != GL_TRUE)
        return false;

    p->x = (GLfloat) wx;
    p->y = (GLfloat) wy;
    p->z = (GLfloat) wz;
    return true;
}

static void mouse(int botao, int estado, int x, int y)
{
    if (botao != GLUT_LEFT_BUTTON || estado != GLUT_DOWN || animando())
        return;

    Ponto p;
    int i, j;
    if (!pontoClicado(x, y, &p) || !tabuleiroCasaSobPonto(&p, &i, &j))
        return;

    Peca *peca = tabuleiroPecaEm(&jogo.tabuleiro, i, j);

    if (peca != NULL) {
        /* seleciona uma peça do jogador da vez */
        if (peca->tipo == jogo.jogadorDaRodada)
            jogo.selecao = peca;
    } else if (jogo.selecao != NULL) {
        tentaMover(i, j);
    }

    glutPostRedisplay();
}

static void redimensiona(int largura, int altura)
{
    jogo.largura = largura;
    jogo.altura = altura > 0 ? altura : 1;

    glViewport(0, 0, jogo.largura, jogo.altura);
    aplicaProjecao();
}

/* ---- inicialização ------------------------------------------------------ */

static void configuraGL(void)
{
    glClearColor(0.3f, 0.1f, 0.2f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   MAT_AMBIENTE);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   MAT_DIFUSA);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  MAT_ESPECULAR);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, MAT_BRILHO);

    glShadeModel(GL_SMOOTH);
}

bool jogoInit(int *argc, char **argv)
{
    glutInit(argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(LARGURA_INICIAL, ALTURA_INICIAL);
    glutCreateWindow(TITULO_JANELA);

    configuraGL();

    jogo.largura = LARGURA_INICIAL;
    jogo.altura = ALTURA_INICIAL;
    jogo.anguloX = 50.0f;
    jogo.anguloY = 0.0f;
    jogo.luz[0] = 0.0f;
    jogo.luz[1] = 7.0f;
    jogo.luz[2] = 7.0f;
    jogo.luz[3] = 0.0f;   /* luz direcional */

    if (!tabuleiroCarregaRecursos(&jogo.tabuleiro))
        return false;

    tabuleiroReinicia(&jogo.tabuleiro);
    jogo.jogadorDaRodada = LEBRE;
    jogo.selecao = &jogo.tabuleiro.pecas[0];   /* a lebre já começa selecionada */
    jogo.numeroAnimacoes = 0;
    atualizaTitulo();

    glutDisplayFunc(desenha);
    glutReshapeFunc(redimensiona);
    glutKeyboardFunc(teclado);
    glutSpecialFunc(tecladoEspecial);
    glutMouseFunc(mouse);

    return true;
}

void jogoExecuta(void)
{
    glutMainLoop();
}
