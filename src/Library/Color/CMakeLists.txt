cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_COLOR_SOURCES
        Colorf.cpp
        Color.cpp
        HsvColorf.cpp)

set(LIBRARY_COLOR_HEADERS
        Color.h
        Colorf.h
        ColorTable.h
        HsvColorf.h)

add_library(library_color STATIC ${LIBRARY_COLOR_SOURCES} ${LIBRARY_COLOR_HEADERS})
target_link_libraries(library_color PUBLIC utility)
target_check_style(library_color)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_COLOR_SOURCES
            Tests/Color_ut.cpp)

    add_library(test_library_color OBJECT ${TEST_LIBRARY_COLOR_SOURCES})
    target_link_libraries(test_library_color PUBLIC testing_unit library_color)

    target_check_style(test_library_color)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_color)
endif()
