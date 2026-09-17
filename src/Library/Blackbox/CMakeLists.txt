cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_BLACKBOX_SOURCES
        Blackbox.cpp)

set(LIBRARY_BLACKBOX_HEADERS
        Blackbox.h)

add_library(library_blackbox STATIC ${LIBRARY_BLACKBOX_SOURCES} ${LIBRARY_BLACKBOX_HEADERS})
target_link_libraries(library_blackbox PUBLIC utility library_stack_trace PRIVATE library_buildinfo)
target_check_style(library_blackbox)

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_BLACKBOX_SOURCES
            Tests/Blackbox_ut.cpp)

    add_library(test_library_blackbox OBJECT ${TEST_LIBRARY_BLACKBOX_SOURCES})
    target_link_libraries(test_library_blackbox PUBLIC testing_unit library_blackbox)

    target_check_style(test_library_blackbox)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_blackbox)
endif()
