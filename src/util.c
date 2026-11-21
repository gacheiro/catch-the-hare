#include "util.h"

#include <math.h>

#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif

float utilDistancia(const Ponto *a, const Ponto *b)
{
    float dx = a->x - b->x;
    float dy = a->y - b->y;
    float dz = a->z - b->z;
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

void utilMatrizSombreamento(const GLfloat plano[4], const GLfloat luz[4])
{
    GLfloat m[4][4];
    GLfloat dot = plano[0] * luz[0] + plano[1] * luz[1]
                + plano[2] * luz[2] + plano[3] * luz[3];

    for (int col = 0; col < 4; col++) {
        for (int lin = 0; lin < 4; lin++) {
            m[lin][col] = (lin == col ? dot : 0.0f) - luz[col] * plano[lin];
        }
    }

    glMultMatrixf(&m[0][0]);
}

void utilRotacionaLuzY(const GLfloat luz[4], GLfloat angulo, GLfloat saida[4])
{
    float rad = angulo * (float) M_PI / 180.0f;
    float c = cosf(rad);
    float s = sinf(rad);

    saida[0] = c * luz[0] - s * luz[2];
    saida[1] = luz[1];
    saida[2] = s * luz[0] + c * luz[2];
    saida[3] = luz[3];
}

Ponto utilPosicaoSalto(const Ponto *origem, const Ponto *destino,
                       float altura, float t)
{
    Ponto p;
    p.x = origem->x + (destino->x - origem->x) * t;
    p.y = origem->y + (destino->y - origem->y) * t + altura * 4.0f * t * (1.0f - t);
    p.z = origem->z + (destino->z - origem->z) * t;
    return p;
}
