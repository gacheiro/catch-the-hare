#include <stdio.h>
#include <stdlib.h>

#include "jogo.h"

int main(int argc, char **argv)
{
    if (!jogoInit(&argc, argv)) {
        fprintf(stderr, "Falha ao inicializar o jogo.\n");
        return EXIT_FAILURE;
    }

    jogoExecuta();
    return EXIT_SUCCESS;
}
