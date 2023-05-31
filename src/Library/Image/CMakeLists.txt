cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(LIBRARY_IMAGE_SOURCES
        PCX.cpp)

set(LIBRARY_IMAGE_HEADERS
        PCX.h)

add_library(library_image STATIC ${LIBRARY_IMAGE_SOURCES} ${LIBRARY_IMAGE_HEADERS})
target_link_libraries(library_image library_color utility)
target_check_style(library_image)
