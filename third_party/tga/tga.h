/*
 * tga.h -- carregador de texturas TGA.
 *
 * Copyright (c) 2005 David HENRY (licença MIT, ver tga.c).
 */
#ifndef CTH_TGA_H
#define CTH_TGA_H

#if defined(__APPLE__)
#  define GL_SILENCE_DEPRECATION
#  include <OpenGL/gl.h>
#else
#  if defined(_WIN32)
#    include <windows.h>
#  endif
#  include <GL/gl.h>
#endif

/*
 * Lê um arquivo .tga e o envia ao OpenGL como textura 2D.
 * Retorna o id da textura, ou 0 em caso de erro.
 * Requer um contexto OpenGL ativo.
 */
GLuint loadTGATexture(const char *filename);

#endif
