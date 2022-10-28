cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

include_directories("${CMAKE_SOURCE_DIR}/test")

set(TEST_UTILITY_SOURCES IndexedArray.cpp)

add_library(test_utility STATIC OBJECT ${TEST_UTILITY_SOURCES})
target_compile_definitions(test_utility PRIVATE TESTGROUP=Utility)
target_link_libraries(test_utility utility)

target_check_style(test_utility)
