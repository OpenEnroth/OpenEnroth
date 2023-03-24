cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

if(ENABLE_TESTS)
    set(TESTING_EXTENSIONS_SOURCES
            ThrowingAssertions.cpp)

    set(TESTING_EXTENSIONS_HEADERS
            ThrowingAssertions.h)

    add_library(testing_extensions ${TESTING_EXTENSIONS_SOURCES} ${TESTING_EXTENSIONS_HEADERS})
    target_link_libraries(testing_extensions GTest::gtest)

    target_check_style(testing_extensions)
endif()
