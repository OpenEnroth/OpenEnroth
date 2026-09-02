cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_VID_SOURCES
        VidReader.cpp
        VidSnapshots.cpp)

set(LIBRARY_VID_HEADERS
        VidReader.h
        VidSnapshots.h)

add_library(library_vid STATIC ${LIBRARY_VID_SOURCES} ${LIBRARY_VID_HEADERS})
target_link_libraries(library_vid PUBLIC library_binary library_snapshots utility)
target_check_style(library_vid)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_VID_SOURCES
            Tests/VidReader_ut.cpp)

    add_library(test_library_vid OBJECT ${TEST_LIBRARY_VID_SOURCES})
    target_link_libraries(test_library_vid PUBLIC testing_unit library_vid)

    target_check_style(test_library_vid)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_vid)
endif()
