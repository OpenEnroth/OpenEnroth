cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(LIBRARY_COLOR_SOURCES
        Color.cpp)

set(LIBRARY_COLOR_HEADERS
        Color.h)

add_library(library_color STATIC ${LIBRARY_COLOR_SOURCES} ${LIBRARY_COLOR_HEADERS})
target_link_libraries(library_color)
target_check_style(library_color)
