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

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_COMPRESSION_SOURCES
            Tests/Compression_ut.cpp)

    add_library(test_library_compression OBJECT ${TEST_LIBRARY_COMPRESSION_SOURCES})
    target_link_libraries(test_library_compression PUBLIC testing_unit library_compression)

    target_check_style(test_library_compression)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_compression)
endif()
