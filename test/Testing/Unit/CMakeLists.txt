cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

if(ENABLE_TESTS)
    set(UNIT_TEST_SOURCES UnitTest.cpp)
    set(UNIT_TEST_HEADERS UnitTest.h)

    add_library(unit_test ${UNIT_TEST_SOURCES} ${UNIT_TEST_HEADERS})
    target_link_libraries(unit_test GTest::gtest)
    target_compile_definitions(unit_test PRIVATE TEST_GROUP=None)

    target_check_style(unit_test)
endif()
