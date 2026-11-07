cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(BIN_FONTGEN_SOURCES
        FontGen.cpp
        FontGenOptions.cpp)

set(BIN_FONTGEN_HEADERS
        FontGenOptions.h)

if(NOT OE_BUILD_PLATFORM STREQUAL "android")
    add_executable(FontGen ${BIN_FONTGEN_SOURCES} ${BIN_FONTGEN_HEADERS})
    target_link_libraries(FontGen PUBLIC
            library_font
            library_lod
            library_lod_formats
            library_filesystem_directory
            library_cli)
    target_check_style(FontGen)
endif()
