#include "recursos.h"

#include <stdio.h>
#include <stdlib.h>

#include "tga.h"

static bool existe(const char *caminho)
{
    FILE *fp = fopen(caminho, "rb");
    if (fp == NULL)
        return false;
    fclose(fp);
    return true;
}

static bool tenta(const char *base, const char *relativo, char *saida, size_t tamanho)
{
    if (base == NULL || base[0] == '\0')
        return false;

    int n = snprintf(saida, tamanho, "%s/%s", base, relativo);
    return n > 0 && (size_t) n < tamanho && existe(saida);
}

bool recursosCaminho(const char *relativo, char *saida, size_t tamanho)
{
    const char *bases[] = {
        getenv("CTH_ASSETS_DIR"),
        "assets",
#ifdef CTH_ASSETS_DIR_FONTE
        CTH_ASSETS_DIR_FONTE,
#endif
#ifdef CTH_ASSETS_DIR_INSTALACAO
        CTH_ASSETS_DIR_INSTALACAO,
#endif
    };

    for (size_t k = 0; k < sizeof bases / sizeof bases[0]; k++) {
        if (tenta(bases[k], relativo, saida, tamanho))
            return true;
    }

    fprintf(stderr, "Recurso nao encontrado: %s "
                    "(defina CTH_ASSETS_DIR com o caminho da pasta assets)\n",
            relativo);
    return false;
}

GLuint recursosCarregaTextura(const char *relativo)
{
    char caminho[1024];
    if (!recursosCaminho(relativo, caminho, sizeof caminho))
        return 0;

    GLuint id = loadTGATexture(caminho);
    if (id == 0)
        fprintf(stderr, "Nao foi possivel carregar a textura %s\n", caminho);
    return id;
}
