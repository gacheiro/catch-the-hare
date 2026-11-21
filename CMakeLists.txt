cmake_minimum_required(VERSION 3.16)

project(CatchTheHare
    VERSION 1.0.0
    DESCRIPTION "Jogo de tabuleiro 3D Catch the Hare (caca a lebre) em OpenGL"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Tipo de build" FORCE)
endif()

option(CTH_BUILD_GAME  "Compila o jogo (requer OpenGL e GLUT)" ON)
option(CTH_BUILD_TESTS "Compila os testes" ON)

include(GNUInstallDirs)

if(MSVC)
    set(CTH_WARNINGS /W4)
else()
    set(CTH_WARNINGS -Wall -Wextra -Wpedantic)
endif()

# --- Regras do jogo (sem dependências) --------------------------------------
add_library(cth_regras STATIC src/regras.c)
target_include_directories(cth_regras PUBLIC src)
target_compile_options(cth_regras PRIVATE ${CTH_WARNINGS})

# --- Jogo ---------------------------------------------------------------------
if(CTH_BUILD_GAME)
    set(OpenGL_GL_PREFERENCE GLVND)
    find_package(OpenGL REQUIRED)
    find_package(GLUT REQUIRED)

    add_library(tga STATIC third_party/tga/tga.c)
    target_include_directories(tga PUBLIC third_party/tga)
    target_link_libraries(tga PUBLIC OpenGL::GL)
    # código de terceiros: não falha o build por avisos
    set_target_properties(tga PROPERTIES COMPILE_WARNING_AS_ERROR OFF)

    add_executable(catch-the-hare
        src/main.c
        src/jogo.c
        src/modelo.c
        src/recursos.c
        src/tabuleiro.c
        src/util.c
    )
    target_compile_options(catch-the-hare PRIVATE ${CTH_WARNINGS})
    target_compile_definitions(catch-the-hare PRIVATE
        CTH_ASSETS_DIR_FONTE="${PROJECT_SOURCE_DIR}/assets"
        CTH_ASSETS_DIR_INSTALACAO="${CMAKE_INSTALL_FULL_DATADIR}/catch-the-hare/assets"
    )
    target_link_libraries(catch-the-hare PRIVATE
        cth_regras
        tga
        GLUT::GLUT
        OpenGL::GL
        OpenGL::GLU
    )
    if(NOT WIN32)
        target_link_libraries(catch-the-hare PRIVATE m)
    endif()

    install(TARGETS catch-the-hare RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(DIRECTORY assets/ DESTINATION ${CMAKE_INSTALL_DATADIR}/catch-the-hare/assets)
endif()

# --- Testes -------------------------------------------------------------------
if(CTH_BUILD_TESTS)
    enable_testing()
    add_executable(test_regras tests/test_regras.c)
    target_link_libraries(test_regras PRIVATE cth_regras)
    target_compile_options(test_regras PRIVATE ${CTH_WARNINGS})
    add_test(NAME regras COMMAND test_regras)
endif()
