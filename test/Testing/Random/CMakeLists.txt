cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

if(ENABLE_TESTS)
    set(TEST_RANDOM_SOURCES TestRandomEngine.cpp)
    set(TEST_RANDOM_HEADERS TestRandomEngine.h)

    add_library(test_random ${TEST_RANDOM_SOURCES} ${TEST_RANDOM_HEADERS})
    target_link_libraries(test_random utility)

    target_check_style(test_random)
endif()
