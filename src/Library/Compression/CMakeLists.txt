cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(LIBRARY_COMPRESSION_SOURCES
        Compression.cpp)

set(LIBRARY_COMPRESSION_HEADERS
        Compression.h)

add_library(library_compression STATIC ${LIBRARY_COMPRESSION_SOURCES} ${LIBRARY_COMPRESSION_HEADERS})
target_link_libraries(library_compression utility ${ZLIB_LIBRARIES})
target_include_directories(library_compression PRIVATE ${ZLIB_INCLUDE_DIRS})
target_check_style(library_compression)

message(VERBOSE "ZLIB_LIBRARIES: ${ZLIB_LIBRARIES}")
