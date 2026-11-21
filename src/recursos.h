/*
 * Localização dos arquivos de recursos (modelos e texturas).
 *
 * A pasta de recursos é procurada, nesta ordem:
 *   1. na variável de ambiente CTH_ASSETS_DIR;
 *   2. em ./assets (diretório de trabalho atual);
 *   3. na pasta assets/ do código-fonte (definida na compilação);
 *   4. na pasta de instalação (definida na compilação).
 */
#ifndef CTH_RECURSOS_H
#define CTH_RECURSOS_H

#include <stdbool.h>
#include <stddef.h>

#include "plataforma_gl.h"

/* Monta em `saida` o caminho completo do recurso `relativo`
 * (ex.: "models/peca.obj"). Retorna false se não for encontrado. */
bool recursosCaminho(const char *relativo, char *saida, size_t tamanho);

/* Carrega uma textura .tga da pasta de recursos. Retorna 0 em caso de erro. */
GLuint recursosCarregaTextura(const char *relativo);

#endif
