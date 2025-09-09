cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_MAGIC_SOURCES
        Magic.cpp)

set(LIBRARY_MAGIC_HEADERS
        Magic.h
        MagicEnums.h)

add_library(library_magic STATIC ${LIBRARY_MAGIC_SOURCES} ${LIBRARY_MAGIC_HEADERS})
target_link_libraries(library_magic PUBLIC library_lod_formats library_image library_lod library_snd library_vid)
target_check_style(library_magic)
