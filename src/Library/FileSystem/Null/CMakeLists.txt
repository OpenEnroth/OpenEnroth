cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_FILESYSTEM_NULL_SOURCES
        NullFileSystem.cpp)

set(LIBRARY_FILESYSTEM_NULL_HEADERS
        NullFileSystem.h)

add_library(library_filesystem_null STATIC ${LIBRARY_FILESYSTEM_NULL_SOURCES} ${LIBRARY_FILESYSTEM_NULL_HEADERS})
target_link_libraries(library_filesystem_null PUBLIC library_filesystem_interface utility)
target_check_style(library_filesystem_null)
