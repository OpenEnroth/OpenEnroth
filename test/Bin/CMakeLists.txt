cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

# This is shared by GameTest & RetraceTest
ExternalProject_Add(OpenEnroth_TestData
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/test_data_tmp
        GIT_REPOSITORY https://github.com/OpenEnroth/OpenEnroth_TestData.git
        GIT_TAG 2aaaefa4b53a7859b7f66085ad7b917fd3b01bfc
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/test_data
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND "")

set(OE_TESTDATA_PATH ${CMAKE_CURRENT_BINARY_DIR}/test_data/data)

add_subdirectory(GameTest)
add_subdirectory(RetraceTest)
add_subdirectory(UnitTest)

# A target to build & run all tests in headless parallel mode.
add_custom_target(Run_AllTests_Headless_Parallel
        DEPENDS Run_UnitTest Run_GameTest_Headless_Parallel Run_RetraceTest_Headless_Parallel)
