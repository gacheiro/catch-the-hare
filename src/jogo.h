/*
 * Laço principal do jogo: estado da partida, câmera, entrada e renderização.
 */
#ifndef CTH_JOGO_H
#define CTH_JOGO_H

#include <stdbool.h>

/* Cria a janela, carrega os recursos e registra os callbacks do GLUT. */
bool jogoInit(int *argc, char **argv);

/* Entra no laço de eventos do GLUT (não retorna). */
void jogoExecuta(void);

#endif
