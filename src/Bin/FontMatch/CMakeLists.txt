cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(BIN_FONTMATCH_SOURCES
        FontMatch.cpp
        FontMatchOptions.cpp)

set(BIN_FONTMATCH_HEADERS
        FontMatchOptions.h)

if(NOT OE_BUILD_PLATFORM STREQUAL "android")
    add_executable(FontMatch ${BIN_FONTMATCH_SOURCES} ${BIN_FONTMATCH_HEADERS})
    target_link_libraries(FontMatch PUBLIC
            library_font
            library_lod
            library_lod_formats
            library_image
            library_cli
            freetype)
    target_check_style(FontMatch)
endif()
