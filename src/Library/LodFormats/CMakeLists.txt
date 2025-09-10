cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_LOD_FORMATS_SOURCES
        LodFormats.cpp
        LodFormatSnapshots.cpp)

set(LIBRARY_LOD_FORMATS_HEADERS
        LodFont.h
        LodFormats.h
        LodFormatSnapshots.h
        LodImage.h
        LodSprite.h)

add_library(library_lod_formats STATIC ${LIBRARY_LOD_FORMATS_SOURCES} ${LIBRARY_LOD_FORMATS_HEADERS})
target_link_libraries(library_lod_formats PUBLIC library_serialization library_binary library_snapshots library_compression utility)
target_check_style(library_lod_formats)
