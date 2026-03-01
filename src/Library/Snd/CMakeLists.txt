cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_SND_SOURCES
        SndReader.cpp
        SndSnapshots.cpp)

set(LIBRARY_SND_HEADERS
        SndReader.h
        SndSnapshots.h)

add_library(library_snd STATIC ${LIBRARY_SND_SOURCES} ${LIBRARY_SND_HEADERS})
target_link_libraries(library_snd PUBLIC library_binary library_snapshots library_compression utility)
target_check_style(library_snd)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_SND_SOURCES
            Tests/SndReader_ut.cpp)

    add_library(test_library_snd OBJECT ${TEST_LIBRARY_SND_SOURCES})
    target_link_libraries(test_library_snd PUBLIC testing_unit library_snd)

    target_check_style(test_library_snd)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_snd)
endif()
