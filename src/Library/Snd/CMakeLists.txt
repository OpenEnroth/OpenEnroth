cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_SND_SOURCES
        SndReader.cpp
        SndSnapshots.cpp)

set(LIBRARY_SND_HEADERS
        SndReader.h
        SndSnapshots.h)

add_library(library_snd STATIC ${LIBRARY_SND_SOURCES} ${LIBRARY_SND_HEADERS})
target_link_libraries(library_snd PUBLIC library_binary library_snapshots library_compression utility)
target_check_style(library_snd)
