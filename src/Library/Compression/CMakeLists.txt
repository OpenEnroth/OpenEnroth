cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(COMPRESSION_SOURCES Compression.cpp)

set(COMPRESSION_HEADERS Compression.h)

add_library(compression STATIC ${COMPRESSION_SOURCES} ${COMPRESSION_HEADERS})
target_link_libraries(compression utility ${ZLIB_LIBRARIES})
target_include_directories(compression PRIVATE ${ZLIB_INCLUDE_DIRS})
target_check_style(compression)

message(VERBOSE "ZLIB_LIBRARIES: ${ZLIB_LIBRARIES}")
