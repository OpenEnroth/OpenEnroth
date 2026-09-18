cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(INTEGRATION_TEST_SOURCES
        IntegrationTest.cpp
        IntegrationTestMain.cpp
        IntegrationTestOptions.cpp
        IntegrationTests.cpp)
set(INTEGRATION_TEST_HEADERS
        IntegrationTest.h
        IntegrationTestOptions.h)

add_executable(OpenEnroth_IntegrationTest ${INTEGRATION_TEST_SOURCES} ${INTEGRATION_TEST_HEADERS})
target_link_libraries(OpenEnroth_IntegrationTest PUBLIC testing_extensions library_cli library_stack_trace utility GTest::gtest)

target_check_style(OpenEnroth_IntegrationTest)

add_custom_target(Run_IntegrationTest
        OpenEnroth_IntegrationTest --binary-path $<TARGET_FILE:OpenEnroth>
        DEPENDS OpenEnroth_IntegrationTest OpenEnroth
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        USES_TERMINAL) # USES_TERMINAL makes the command print progress as it goes.
