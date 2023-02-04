cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(LOD_SOURCES LodReader.cpp)

set(LOD_HEADERS 
                LodReader.h
                LodVersion.h)

add_library(lod STATIC ${LOD_SOURCES} ${LOD_HEADERS})
target_link_libraries(lod compression utility)
target_check_style(lod)
