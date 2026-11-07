cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(BIN_FONTTOOL_SOURCES
        Diff.cpp
        FontFiles.cpp
        FontTool.cpp
        FontToolOptions.cpp
        FreeType.cpp
        Generate.cpp
        Render.cpp
        Search.cpp
        Shadow.cpp)

set(BIN_FONTTOOL_HEADERS
        Diff.h
        FontFiles.h
        FontToolOptions.h
        FreeType.h
        Generate.h
        Render.h
        Search.h
        Shadow.h)

if(NOT OE_BUILD_PLATFORM STREQUAL "android")
    add_executable(FontTool ${BIN_FONTTOOL_SOURCES} ${BIN_FONTTOOL_HEADERS})
    target_link_libraries(FontTool PUBLIC
            bin_lodtool_archive
            library_font
            library_lod_formats
            library_image
            library_magic
            library_cli
            utility
            freetype)
    target_check_style(FontTool)
endif()
