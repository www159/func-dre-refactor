cmake_minimum_required(VERSION 3.4)

# lex yacc
find_package(BISON)
BISON_TARGET(
    parser_y
    parser.y
    ${CMAKE_CURRENT_BINARY_DIR}/parser.c
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.h
)

find_package(FLEX)
FLEX_TARGET(
    lexer_l
    lexer.l
    ${CMAKE_CURRENT_BINARY_DIR}/lexer.c
)

ADD_FLEX_BISON_DEPENDENCY(lexer_l parser_y)

# use glib
find_package(PkgConfig REQUIRED)

PKG_CHECK_MODULES(GLIB REQUIRED glib-2.0)


add_subdirectory(ast)

add_library(
    parser
    parser.c
    lexer.c
)

target_include_directories(
    parser
    PRIVATE
    ast  
)

target_link_libraries(
    parser 
    PUBLIC
    ast
    ${FLEX_LIBRARIES}
)