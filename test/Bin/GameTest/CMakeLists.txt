cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(GAME_TEST_MAIN_SOURCES
        GameTestMain.cpp
        GameTestOptions.cpp
        GameTests_0000.cpp
        GameTests_0500.cpp
        GameTests_1000.cpp
        GameTests_1500.cpp
        GameTests_2000.cpp)
set(GAME_TEST_MAIN_HEADERS
        GameTestOptions.h)

add_executable(OpenEnroth_GameTest ${GAME_TEST_MAIN_SOURCES} ${GAME_TEST_MAIN_HEADERS})
target_link_libraries(OpenEnroth_GameTest PUBLIC application testing_game library_cli library_platform_main library_stack_trace)

target_check_style(OpenEnroth_GameTest)

add_custom_target(Run_GameTest
        OpenEnroth_GameTest --test-path ${OE_TESTDATA_PATH}
        DEPENDS OpenEnroth_GameTest OpenEnroth_TestData
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        USES_TERMINAL) # USES_TERMINAL makes the command print progress as it goes.

add_custom_target(Run_GameTest_Headless
        OpenEnroth_GameTest --test-path ${OE_TESTDATA_PATH} --headless
        DEPENDS OpenEnroth_GameTest OpenEnroth_TestData
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        USES_TERMINAL)

add_custom_target(Run_GameTest_Parallel
        Python::Interpreter ${CMAKE_SOURCE_DIR}/thirdparty/gtest_parallel/gtest-parallel --print_test_times
            ${CMAKE_CURRENT_BINARY_DIR}/OpenEnroth_GameTest -- --test-path ${OE_TESTDATA_PATH}
        DEPENDS OpenEnroth_GameTest OpenEnroth_TestData
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        USES_TERMINAL)

add_custom_target(Run_GameTest_Headless_Parallel
        Python::Interpreter ${CMAKE_SOURCE_DIR}/thirdparty/gtest_parallel/gtest-parallel --print_test_times
            ${CMAKE_CURRENT_BINARY_DIR}/OpenEnroth_GameTest -- --test-path ${OE_TESTDATA_PATH} --headless
        DEPENDS OpenEnroth_GameTest OpenEnroth_TestData
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        USES_TERMINAL)
