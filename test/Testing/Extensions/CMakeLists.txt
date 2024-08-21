cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

if(OE_BUILD_TESTS)
    set(TESTING_EXTENSIONS_SOURCES
            ThrowingAssertions.cpp
            ScopedTestFile.cpp
            ScopedTestFileSlot.cpp)

    set(TESTING_EXTENSIONS_HEADERS
            ThrowingAssertions.h
            ExpectExtensions.h
            ScopedTestFile.h
            ScopedTestFileSlot.h)

    add_library(testing_extensions ${TESTING_EXTENSIONS_SOURCES} ${TESTING_EXTENSIONS_HEADERS})
    target_link_libraries(testing_extensions PUBLIC GTest::gtest)

    target_check_style(testing_extensions)
endif()
