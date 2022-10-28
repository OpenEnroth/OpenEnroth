cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(ENABLE_TESTS OFF CACHE BOOL "Enable tests")

if(ENABLE_TESTS)
    message(STATUS "Tests have been enabled")

    add_subdirectory(Utility)

    set(TEST_SOURCES test.cpp)
    set(TEST_HEADERS test.h)

    add_executable(World_of_Might_and_Magic_test ${TEST_SOURCES} ${TEST_HEADERS})
    target_fix_libcxx_assertions(World_of_Might_and_Magic_test)
    target_link_libraries(World_of_Might_and_Magic_test GTest::gtest test_utility)

    add_custom_target(test World_of_Might_and_Magic_test
                      DEPENDS World_of_Might_and_Magic_test
                      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
