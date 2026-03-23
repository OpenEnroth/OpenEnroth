cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

add_subdirectory(GameTest)
add_subdirectory(RetraceTest)
add_subdirectory(UnitTest)

# A target to build & run all tests in headless parallel mode.
add_custom_target(Run_AllTests_Headless_Parallel
        DEPENDS Run_UnitTest Run_GameTest_Headless_Parallel Run_RetraceTest_Headless_Parallel)
