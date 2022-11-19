cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

if(ENABLE_TESTS)
    set(UNIT_TEST_MAIN_SOURCES UnitTestMain.cpp)

    add_executable(World_of_Might_and_Magic_UnitTest ${UNIT_TEST_MAIN_SOURCES})
    target_fix_libcxx_assertions(World_of_Might_and_Magic_UnitTest)
    target_link_libraries(World_of_Might_and_Magic_UnitTest unit_test)

    add_custom_target(UnitTest World_of_Might_and_Magic_UnitTest
        DEPENDS World_of_Might_and_Magic_UnitTest
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

    target_check_style(World_of_Might_and_Magic_UnitTest)
endif()
