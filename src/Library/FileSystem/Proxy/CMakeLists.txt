cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_FILESYSTEM_PROXY_SOURCES
        ProxyFileSystem.cpp)

set(LIBRARY_FILESYSTEM_PROXY_HEADERS
        ProxyFileSystem.h)

add_library(library_filesystem_proxy STATIC ${LIBRARY_FILESYSTEM_PROXY_SOURCES} ${LIBRARY_FILESYSTEM_PROXY_HEADERS})
target_link_libraries(library_filesystem_proxy PUBLIC library_filesystem_interface utility)
target_check_style(library_filesystem_proxy)
