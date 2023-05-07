cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(LIBRARY_BINARY_SOURCES
        BinaryExceptions.cpp)

set(LIBRARY_BINARY_HEADERS
        ArraySerialization.h
        BinaryExceptions.h
        BinarySerialization.h
        BlobSerialization.h
        MemCopySerialization.h)

add_library(library_binary STATIC ${LIBRARY_BINARY_SOURCES} ${LIBRARY_BINARY_HEADERS})
target_link_libraries(library_binary utility)
target_check_style(library_binary)


