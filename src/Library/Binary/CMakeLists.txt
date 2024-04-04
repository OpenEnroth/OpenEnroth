cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_BINARY_SOURCES
        BinaryExceptions.cpp)

set(LIBRARY_BINARY_HEADERS
        BinaryConcepts.h
        BinaryExceptions.h
        BinarySerialization.h
        BinaryTags.h
        BlobSerialization.h
        ContainerSerialization.h
        MemCopySerialization.h)

add_library(library_binary STATIC ${LIBRARY_BINARY_SOURCES} ${LIBRARY_BINARY_HEADERS})
target_link_libraries(library_binary PUBLIC utility)
target_check_style(library_binary)


