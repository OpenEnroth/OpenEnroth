cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

if(ENABLE_TESTS)
    set(TEST_SOURCES UnitTestMain.cpp)
    set(TEST_HEADERS UnitTest.h)

    add_executable(World_of_Might_and_Magic_test ${TEST_SOURCES} ${TEST_HEADERS})
    target_fix_libcxx_assertions(World_of_Might_and_Magic_test)
    target_link_libraries(World_of_Might_and_Magic_test GTest::gtest)

    add_custom_target(test World_of_Might_and_Magic_test
        DEPENDS World_of_Might_and_Magic_test
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    target_check_style(World_of_Might_and_Magic_test)
endif()
