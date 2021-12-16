cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

set(LOD_SOURCES
    Lod.cpp
    LodUtils.cpp
)

set(LOD_HEADERS
    Directory.h
    File.h
    Header.h
    Lod.h
    LodIconsBitmaps.h
    LodSprites.h
    LodUtils.h
    LodWriter.h
    Version.h
)


add_library(lod STATIC ${LOD_SOURCES} ${LOD_HEADERS})
target_check_style(lod)
