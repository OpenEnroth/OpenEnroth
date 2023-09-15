cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(LIBRARY_COLOR_SOURCES
        Colorf.cpp
        HsvColorf.cpp)

set(LIBRARY_COLOR_HEADERS
        Color.h
        Colorf.h
        ColorTable.h
        HsvColorf.h)

add_library(library_color STATIC ${LIBRARY_COLOR_SOURCES} ${LIBRARY_COLOR_HEADERS})
target_link_libraries(library_color PUBLIC utility)
target_check_style(library_color)
