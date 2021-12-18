cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

set(LOD_SOURCES
    Reader.cpp
    Utils.cpp
)

set(LOD_HEADERS
    Directory.h
    File.h
    Header.h
    LodIconsBitmaps.h
    LodSprites.h
    Reader.h
    Utils.h
    Version.h
    Writer.h
)


add_library(lod STATIC ${LOD_SOURCES} ${LOD_HEADERS})
target_check_style(lod)
