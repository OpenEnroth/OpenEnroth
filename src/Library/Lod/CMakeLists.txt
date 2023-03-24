cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(LIBRARY_LOD_SOURCES
        LodReader.cpp)

set(LIBRARY_LOD_HEADERS
        LodReader.h
        LodVersion.h)

add_library(library_lod STATIC ${LIBRARY_LOD_SOURCES} ${LIBRARY_LOD_HEADERS})
target_link_libraries(library_lod library_compression utility)
target_check_style(library_lod)
