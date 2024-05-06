cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_FILESYSTEM_MERGING_SOURCES
        MergingFileSystem.cpp)

set(LIBRARY_FILESYSTEM_MERGING_HEADERS
        MergingFileSystem.h)

add_library(library_filesystem_merging STATIC ${LIBRARY_FILESYSTEM_MERGING_SOURCES} ${LIBRARY_FILESYSTEM_MERGING_HEADERS})
target_link_libraries(library_filesystem_merging PUBLIC library_filesystem_interface utility)
target_check_style(library_filesystem_merging)
