cmake_minimum_required(VERSION 3.4)

# use glib
find_package(PkgConfig REQUIRED)

PKG_CHECK_MODULES(GLIB REQUIRED glib-2.0)

set(LIB_NAME ast)

add_library(
    ast
    ast.c
    symbol.c
    emit.c
    yacc.c
    builtin_func.c
    simplify.c
    init.c
    help.c
)

target_include_directories(
    ast
    PUBLIC
    ${GLIB_INCLUDE_DIRS}    
)

target_link_libraries(
    ast
    PUBLIC
    ${GLIB_LIBRARIES}
)