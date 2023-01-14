cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(LOD_SOURCES
    ""
)

set(LOD_HEADERS
    Lod.h
    Version.h
)


add_library(lod STATIC ${LOD_SOURCES} ${LOD_HEADERS})
target_check_style(lod)
