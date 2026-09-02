cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(BIN_LODTOOL_SOURCES
        ArchiveReader.cpp
        LodTool.cpp
        LodToolOptions.cpp)

set(BIN_LODTOOL_HEADERS
        ArchiveReader.h
        LodToolOptions.h)

if(NOT OE_BUILD_PLATFORM STREQUAL "android")
    add_executable(LodTool ${BIN_LODTOOL_SOURCES} ${BIN_LODTOOL_HEADERS})
    target_link_libraries(LodTool PUBLIC
            library_lod
            library_lod_formats
            library_image
            library_magic
            library_filesystem_directory
            library_cli)
    target_check_style(LodTool)
endif()
