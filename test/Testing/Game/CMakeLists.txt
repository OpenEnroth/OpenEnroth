cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

if(ENABLE_TESTS)
    set(GAME_TEST_SOURCES GameTest.cpp
                          GameWrapper.cpp)
    set(GAME_TEST_HEADERS GameTest.h
                          GameWrapper.h)

    add_library(game_test ${GAME_TEST_SOURCES} ${GAME_TEST_HEADERS})
    target_link_libraries(game_test game test_engine test_extensions GTest::gtest)
    target_compile_definitions(game_test PRIVATE TEST_GROUP=None)

    target_check_style(game_test)
endif()
