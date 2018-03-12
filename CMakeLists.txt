cmake_minimum_required (VERSION 2.9)

project (vouwmat)

#set( CMAKE_VERBOSE_MAKEFILE ON )

list (APPEND CMAKE_C_FLAGS "-g -O2 -std=c99")

add_library (vouw STATIC
    src/vouw/matrix.c
    src/vouw/pattern.c
    src/vouw/region.c
    src/vouw/vouw.c
    src/vouw/list_sort.c)

add_executable (image-vouw 
    src/image-vouw/main.c)

target_link_libraries (vouw "-lm")
target_link_libraries (image-vouw vouw)
target_include_directories (vouw PRIVATE "include")
target_include_directories (image-vouw PRIVATE "include")
