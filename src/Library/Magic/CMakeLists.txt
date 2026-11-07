cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_MAGIC_SOURCES
        Magic.cpp
        MagicEnumFunctions.cpp)

set(LIBRARY_MAGIC_HEADERS
        Magic.h
        MagicEnums.h
        MagicEnumFunctions.h)

add_library(library_magic STATIC ${LIBRARY_MAGIC_SOURCES} ${LIBRARY_MAGIC_HEADERS})
target_link_libraries(library_magic PUBLIC library_lod_formats library_image library_lod library_snd library_vid)
target_check_style(library_magic)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_MAGIC_SOURCES
            Tests/Magic_ut.cpp)

    add_library(test_library_magic OBJECT ${TEST_LIBRARY_MAGIC_SOURCES})
    target_link_libraries(test_library_magic PUBLIC testing_unit library_magic)

    target_check_style(test_library_magic)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_magic)
endif()
