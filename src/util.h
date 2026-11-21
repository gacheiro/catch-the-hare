/*
 * Funções matemáticas e auxiliares.
 */
#ifndef CTH_UTIL_H
#define CTH_UTIL_H

#include "plataforma_gl.h"

typedef struct {
    GLfloat x, y, z;
} Ponto;

/* Distância euclidiana entre dois pontos 3D. */
float utilDistancia(const Ponto *a, const Ponto *b);

/* Multiplica a matriz corrente por uma matriz de projeção de sombra:
 * desenha os modelos achatados sobre `plano` (normal + distância à origem)
 * a partir da luz `luz` (posição homogênea). */
void utilMatrizSombreamento(const GLfloat plano[4], const GLfloat luz[4]);

/* Rotaciona a posição da luz em torno do eixo Y por `angulo` graus. */
void utilRotacionaLuzY(const GLfloat luz[4], GLfloat angulo, GLfloat saida[4]);

/* Posição de um salto em arco entre `origem` e `destino`.
 * `t` varia de 0 a 1; `altura` é a altura máxima acima da reta. */
Ponto utilPosicaoSalto(const Ponto *origem, const Ponto *destino,
                       float altura, float t);

#endif
