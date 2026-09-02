cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(RETRACE_TEST_SOURCES
        RetraceTest.cpp
        RetraceTestMain.cpp
        RetraceTestOptions.cpp)
set(RETRACE_TEST_HEADERS
        RetraceTest.h
        RetraceTestOptions.h)

add_executable(OpenEnroth_RetraceTest ${RETRACE_TEST_SOURCES} ${RETRACE_TEST_HEADERS})
target_link_libraries(OpenEnroth_RetraceTest PUBLIC application GTest::gtest library_cli library_platform_main library_stack_trace)

target_check_style(OpenEnroth_RetraceTest)

add_custom_target(Run_RetraceTest
        OpenEnroth_RetraceTest --test-path ${OE_TESTDATA_PATH}
        DEPENDS OpenEnroth_RetraceTest
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        USES_TERMINAL) # USES_TERMINAL makes the command print progress as it goes.

add_custom_target(Run_RetraceTest_Headless
        OpenEnroth_RetraceTest --test-path ${OE_TESTDATA_PATH} --headless
        DEPENDS OpenEnroth_RetraceTest
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        USES_TERMINAL)

add_custom_target(Run_RetraceTest_Parallel
        Python::Interpreter ${CMAKE_SOURCE_DIR}/thirdparty/gtest_parallel/gtest-parallel --print_test_times
            $<TARGET_FILE:OpenEnroth_RetraceTest> -- --test-path ${OE_TESTDATA_PATH}
        DEPENDS OpenEnroth_RetraceTest
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        USES_TERMINAL)

add_custom_target(Run_RetraceTest_Headless_Parallel
        Python::Interpreter ${CMAKE_SOURCE_DIR}/thirdparty/gtest_parallel/gtest-parallel --print_test_times
            $<TARGET_FILE:OpenEnroth_RetraceTest> -- --test-path ${OE_TESTDATA_PATH} --headless
        DEPENDS OpenEnroth_RetraceTest
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        USES_TERMINAL)
