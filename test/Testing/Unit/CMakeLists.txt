cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

if(OE_BUILD_TESTS)
    set(TESTING_UNIT_SOURCES
            UnitTest.cpp)
    set(TESTING_UNIT_HEADERS
            UnitTest.h)

    add_library(testing_unit ${TESTING_UNIT_SOURCES} ${TESTING_UNIT_HEADERS})
    target_link_libraries(testing_unit PUBLIC testing_extensions GTest::gtest GTest::gmock)

    target_check_style(testing_unit)
endif()
