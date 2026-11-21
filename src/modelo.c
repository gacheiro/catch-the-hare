#include "modelo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

/* ---- vetor dinâmico simples ------------------------------------------- */

typedef struct {
    void *dados;
    size_t tamanho, capacidade, tamanhoElemento;
} Vetor;

static void vetorInit(Vetor *v, size_t tamanhoElemento)
{
    v->dados = NULL;
    v->tamanho = v->capacidade = 0;
    v->tamanhoElemento = tamanhoElemento;
}

static bool vetorAdiciona(Vetor *v, const void *elemento)
{
    if (v->tamanho == v->capacidade) {
        size_t novaCapacidade = v->capacidade ? v->capacidade * 2 : 256;
        void *novo = realloc(v->dados, novaCapacidade * v->tamanhoElemento);
        if (novo == NULL)
            return false;
        v->dados = novo;
        v->capacidade = novaCapacidade;
    }
    memcpy((char *) v->dados + v->tamanho * v->tamanhoElemento,
           elemento, v->tamanhoElemento);
    v->tamanho++;
    return true;
}

static void vetorLibera(Vetor *v)
{
    free(v->dados);
    vetorInit(v, v->tamanhoElemento);
}

/* ---- leitura do .obj ---------------------------------------------------- */

/* índice de um canto de face: posição, textura e normal (-1 = ausente) */
typedef struct {
    int v, vt, vn;
} Canto;

typedef struct {
    Vetor vertices;    /* Ponto */
    Vetor texturas;    /* Ponto (z ignorado) */
    Vetor normais;     /* Ponto */
    Vetor cantos;      /* Canto, 3 por triângulo */
} DadosObj;

/* Converte um índice do .obj (base 1, ou negativo = relativo ao fim)
 * para base 0. Retorna -1 se ausente ou inválido. */
static int resolveIndice(int indice, size_t total)
{
    if (indice > 0 && (size_t) indice <= total)
        return indice - 1;
    if (indice < 0 && (size_t) (-indice) <= total)
        return (int) total + indice;
    return -1;
}

static bool leCanto(const char *texto, const DadosObj *obj, Canto *c)
{
    int v = 0, vt = 0, vn = 0;

    if (sscanf(texto, "%d/%d/%d", &v, &vt, &vn) == 3 ||
        (vt = 0, sscanf(texto, "%d//%d", &v, &vn) == 2) ||
        (vn = 0, sscanf(texto, "%d/%d", &v, &vt) == 2) ||
        (vt = vn = 0, sscanf(texto, "%d", &v) == 1)) {

        c->v  = resolveIndice(v,  obj->vertices.tamanho);
        c->vt = resolveIndice(vt, obj->texturas.tamanho);
        c->vn = resolveIndice(vn, obj->normais.tamanho);
        return c->v >= 0;
    }
    return false;
}

static bool leFace(char *linha, DadosObj *obj)
{
    Canto primeiro, anterior, atual;
    int n = 0;

    /* triangula o polígono em leque: (0, k-1, k) */
    for (char *tok = strtok(linha, " \t\r\n"); tok; tok = strtok(NULL, " \t\r\n")) {
        if (!leCanto(tok, obj, &atual))
            return false;

        if (n == 0) {
            primeiro = atual;
        } else if (n >= 2) {
            if (!vetorAdiciona(&obj->cantos, &primeiro) ||
                !vetorAdiciona(&obj->cantos, &anterior) ||
                !vetorAdiciona(&obj->cantos, &atual))
                return false;
        }
        anterior = atual;
        n++;
    }
    return n >= 3;
}

static void compilaListas(const DadosObj *obj, Modelo *modelo)
{
    const Ponto *vs = obj->vertices.dados;
    const Ponto *vts = obj->texturas.dados;
    const Ponto *vns = obj->normais.dados;
    const Canto *cs = obj->cantos.dados;
    size_t n = obj->cantos.tamanho;

    modelo->listaCompleta = glGenLists(2);
    modelo->listaSimples = modelo->listaCompleta + 1;
    modelo->numeroTriangulos = (int) (n / 3);

    glNewList(modelo->listaCompleta, GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for (size_t k = 0; k < n; k++) {
        if (cs[k].vn >= 0)
            glNormal3f(vns[cs[k].vn].x, vns[cs[k].vn].y, vns[cs[k].vn].z);
        if (cs[k].vt >= 0)
            glTexCoord2f(vts[cs[k].vt].x, vts[cs[k].vt].y);
        glVertex3f(vs[cs[k].v].x, vs[cs[k].v].y, vs[cs[k].v].z);
    }
    glEnd();
    glEndList();

    glNewList(modelo->listaSimples, GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for (size_t k = 0; k < n; k++)
        glVertex3f(vs[cs[k].v].x, vs[cs[k].v].y, vs[cs[k].v].z);
    glEnd();
    glEndList();
}

bool modeloCarregaObj(const char *arquivo, Modelo *modelo)
{
    FILE *fp = fopen(arquivo, "r");
    if (fp == NULL) {
        fprintf(stderr, "Nao foi possivel abrir o modelo %s\n", arquivo);
        return false;
    }

    DadosObj obj;
    vetorInit(&obj.vertices, sizeof(Ponto));
    vetorInit(&obj.texturas, sizeof(Ponto));
    vetorInit(&obj.normais, sizeof(Ponto));
    vetorInit(&obj.cantos, sizeof(Canto));

    char linha[1024];
    int numeroLinha = 0;
    bool ok = true;

    while (ok && fgets(linha, sizeof linha, fp) != NULL) {
        numeroLinha++;
        Ponto p = { 0, 0, 0 };

        if (strncmp(linha, "v ", 2) == 0) {
            ok = sscanf(linha + 2, "%f %f %f", &p.x, &p.y, &p.z) == 3
              && vetorAdiciona(&obj.vertices, &p);
        } else if (strncmp(linha, "vt ", 3) == 0) {
            ok = sscanf(linha + 3, "%f %f", &p.x, &p.y) == 2
              && vetorAdiciona(&obj.texturas, &p);
        } else if (strncmp(linha, "vn ", 3) == 0) {
            ok = sscanf(linha + 3, "%f %f %f", &p.x, &p.y, &p.z) == 3
              && vetorAdiciona(&obj.normais, &p);
        } else if (strncmp(linha, "f ", 2) == 0) {
            ok = leFace(linha + 2, &obj);
        }
        /* outras diretivas (o, s, usemtl, mtllib, comentários) são ignoradas */
    }
    fclose(fp);

    if (ok)
        compilaListas(&obj, modelo);
    else
        fprintf(stderr, "Erro ao ler %s na linha %d\n", arquivo, numeroLinha);

    vetorLibera(&obj.vertices);
    vetorLibera(&obj.texturas);
    vetorLibera(&obj.normais);
    vetorLibera(&obj.cantos);
    return ok;
}

void modeloDesenha(const Modelo *modelo)
{
    glCallList(modelo->listaCompleta);
}

void modeloDesenhaSimples(const Modelo *modelo)
{
    glCallList(modelo->listaSimples);
}

void modeloLibera(Modelo *modelo)
{
    if (modelo->listaCompleta)
        glDeleteLists(modelo->listaCompleta, 2);
    modelo->listaCompleta = modelo->listaSimples = 0;
    modelo->numeroTriangulos = 0;
}
