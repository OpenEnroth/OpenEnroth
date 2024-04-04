cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_COMPRESSION_SOURCES
        Compression.cpp)

set(LIBRARY_COMPRESSION_HEADERS
        Compression.h)

add_library(library_compression STATIC ${LIBRARY_COMPRESSION_SOURCES} ${LIBRARY_COMPRESSION_HEADERS})
target_check_style(library_compression)
target_link_libraries(library_compression
        PUBLIC
        utility
        PRIVATE
        ZLIB::ZLIB)

message(VERBOSE "ZLIB_LIBRARIES: ${ZLIB_LIBRARIES}")
