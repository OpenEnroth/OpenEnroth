cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_FONT_SOURCES
        Font.cpp
        Oef.cpp)

set(LIBRARY_FONT_HEADERS
        Font.h
        Oef.h
        OefSnapshots.h)

add_library(library_font STATIC ${LIBRARY_FONT_SOURCES} ${LIBRARY_FONT_HEADERS})
target_link_libraries(library_font PUBLIC library_image library_binary library_snapshots library_compression utility)
target_check_style(library_font)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_FONT_SOURCES
            Tests/Font_ut.cpp
            Tests/Oef_ut.cpp)

    add_library(test_library_font OBJECT ${TEST_LIBRARY_FONT_SOURCES})
    target_link_libraries(test_library_font PUBLIC testing_unit library_font)

    target_check_style(test_library_font)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_font)
endif()
