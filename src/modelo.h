/*
 * Modelos 3D carregados de arquivos Wavefront .obj.
 *
 * A geometria é compilada em display lists do OpenGL logo após a leitura,
 * então desenhar um modelo é barato mesmo com milhares de faces.
 */
#ifndef CTH_MODELO_H
#define CTH_MODELO_H

#include <stdbool.h>

#include "plataforma_gl.h"

typedef struct {
    GLuint listaCompleta;   /* com normais e coordenadas de textura */
    GLuint listaSimples;    /* só vértices: usada para sombras */
    int numeroTriangulos;
} Modelo;

/* Carrega um arquivo .obj (requer contexto OpenGL ativo).
 * Suporta faces poligonais nos formatos v, v/vt, v//vn e v/vt/vn. */
bool modeloCarregaObj(const char *arquivo, Modelo *modelo);

/* Desenha o modelo com iluminação e textura. */
void modeloDesenha(const Modelo *modelo);

/* Desenha apenas a silhueta do modelo (sem normais nem textura). */
void modeloDesenhaSimples(const Modelo *modelo);

void modeloLibera(Modelo *modelo);

#endif
