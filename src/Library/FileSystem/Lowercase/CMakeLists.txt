cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_FILESYSTEM_LOWERCASE_SOURCES
        LowercaseFileSystem.cpp)

set(LIBRARY_FILESYSTEM_LOWERCASE_HEADERS
        LowercaseFileSystem.h)

add_library(library_filesystem_lowercase STATIC ${LIBRARY_FILESYSTEM_LOWERCASE_SOURCES} ${LIBRARY_FILESYSTEM_LOWERCASE_HEADERS})
target_link_libraries(library_filesystem_lowercase PUBLIC library_filesystem_interface utility)
target_check_style(library_filesystem_lowercase)
