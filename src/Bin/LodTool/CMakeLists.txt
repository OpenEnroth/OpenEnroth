cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(BIN_LODTOOL_SOURCES
        LodTool.cpp
        LodToolOptions.cpp)

set(BIN_LODTOOL_HEADERS
        LodToolOptions.h)

if(NOT OE_BUILD_PLATFORM STREQUAL "android")
    add_library(bin_lodtool_archive STATIC ArchiveReader.cpp ArchiveReader.h)
    target_link_libraries(bin_lodtool_archive PUBLIC library_lod library_vid library_snd library_magic utility)
    target_check_style(bin_lodtool_archive)

    add_executable(LodTool ${BIN_LODTOOL_SOURCES} ${BIN_LODTOOL_HEADERS})
    target_link_libraries(LodTool PUBLIC
            bin_lodtool_archive
            library_lod
            library_font
            library_lod_formats
            library_image
            library_magic
            library_cli
            utility)
    target_check_style(LodTool)
endif()
