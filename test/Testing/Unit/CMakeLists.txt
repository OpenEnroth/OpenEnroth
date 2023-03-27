cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

if(ENABLE_TESTS)
    set(TESTING_UNIT_SOURCES
            UnitTest.cpp)
    set(TESTING_UNIT_HEADERS
            UnitTest.h)

    add_library(testing_unit ${TESTING_UNIT_SOURCES} ${TESTING_UNIT_HEADERS})
    target_link_libraries(testing_unit GTest::gtest)
    target_compile_definitions(testing_unit PRIVATE TEST_GROUP=None)

    target_check_style(testing_unit)
endif()
