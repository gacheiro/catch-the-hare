/*
 * Includes de OpenGL/GLU/GLUT independentes de plataforma.
 */
#ifndef CTH_PLATAFORMA_GL_H
#define CTH_PLATAFORMA_GL_H

#if defined(__APPLE__)
#  define GL_SILENCE_DEPRECATION
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#endif
