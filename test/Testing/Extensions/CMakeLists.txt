cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

if(ENABLE_TESTS)
    set(TEST_EXTENSIONS_SOURCES ThrowingAssertions.cpp)
    set(TEST_EXTENSIONS_HEADERS ThrowingAssertions.h)

    add_library(test_extensions ${TEST_EXTENSIONS_SOURCES} ${TEST_EXTENSIONS_HEADERS})
    target_link_libraries(test_extensions GTest::gtest)

    target_check_style(test_extensions)
endif()
