cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(UNIT_TEST_MAIN_SOURCES
        UnitTestMain.cpp)

add_executable(OpenEnroth_UnitTest ${UNIT_TEST_MAIN_SOURCES})
target_link_libraries(OpenEnroth_UnitTest PUBLIC testing_unit library_stack_trace)

add_custom_target(Run_UnitTest OpenEnroth_UnitTest
        DEPENDS OpenEnroth_UnitTest
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        USES_TERMINAL) # USES_TERMINAL makes the command print progress as it goes.

target_check_style(OpenEnroth_UnitTest)
