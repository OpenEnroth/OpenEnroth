cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_BINARY_SOURCES
        BinaryExceptions.cpp)

set(LIBRARY_BINARY_HEADERS
        BinaryConcepts.h
        BinaryExceptions.h
        BinarySerialization.h
        BinaryTags.h
        BlobSerialization.h
        CommonSerialization.h)

add_library(library_binary STATIC ${LIBRARY_BINARY_SOURCES} ${LIBRARY_BINARY_HEADERS})
target_link_libraries(library_binary PUBLIC utility)
target_check_style(library_binary)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_BINARY_SOURCES Tests/Binary_ut.cpp)

    add_library(test_library_binary OBJECT ${TEST_LIBRARY_BINARY_SOURCES})
    target_link_libraries(test_library_binary PUBLIC testing_unit library_binary)

    target_check_style(test_library_binary)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_binary)
endif()
