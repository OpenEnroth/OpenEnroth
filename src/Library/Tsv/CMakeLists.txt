cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_TSV_SOURCES
        TsvCell.cpp
        TsvReader.cpp)

set(LIBRARY_TSV_HEADERS
        TsvCell.h
        TsvLine.h
        TsvReader.h)

add_library(library_tsv STATIC ${LIBRARY_TSV_SOURCES} ${LIBRARY_TSV_HEADERS})
target_link_libraries(library_tsv PUBLIC library_serialization utility)
target_check_style(library_tsv)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_TSV_SOURCES Tests/TsvReader_ut.cpp)

    add_library(test_library_tsv OBJECT ${TEST_LIBRARY_TSV_SOURCES})
    target_link_libraries(test_library_tsv PUBLIC testing_unit library_tsv)

    target_check_style(test_library_tsv)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_tsv)
endif()
