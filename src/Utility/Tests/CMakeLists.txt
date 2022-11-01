cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(TEST_UTILITY_SOURCES IndexedArray_ut.cpp
                         Segment_ut.cpp)

add_library(test_utility OBJECT ${TEST_UTILITY_SOURCES})
target_compile_definitions(test_utility PRIVATE TESTGROUP=Utility)
target_link_libraries(test_utility utility)

target_check_style(test_utility)

target_link_libraries(World_of_Might_and_Magic_test test_utility)
