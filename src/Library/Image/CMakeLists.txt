cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_IMAGE_SOURCES
        ImageFunctions.cpp
        Pcx.cpp)

set(LIBRARY_IMAGE_HEADERS
        Image.h
        ImageFunctions.h
        Palette.h
        Pcx.h
        Png.h
        Png.cpp)

add_library(library_image STATIC ${LIBRARY_IMAGE_SOURCES} ${LIBRARY_IMAGE_HEADERS})
target_link_libraries(library_image PUBLIC library_color library_geometry utility PRIVATE PNG::PNG)
target_check_style(library_image)
