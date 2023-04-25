cmake_minimum_required(VERSION 3.4)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

# glib debug
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("glib debug macro")
    SET(ENV{G_MESSAGES_DEBUG} "all")
endif()

project(dre)

add_subdirectory(parser)

add_executable(
    dre
    main.c
)

target_include_directories(
    dre
    PRIVATE
    parser
)

target_link_libraries(
    dre
    PRIVATE
    parser
)