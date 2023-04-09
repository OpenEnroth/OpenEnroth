cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

if(ENABLE_TESTS)
    set(TESTING_GAME_SOURCES
            GameTest.cpp
            TestController.cpp)
    set(TESTING_GAME_HEADERS
            GameTest.h
            TestController.h)

    add_library(testing_game ${TESTING_GAME_SOURCES} ${TESTING_GAME_HEADERS})
    target_link_libraries(testing_game application testing_extensions GTest::gtest)
    target_compile_definitions(testing_game PRIVATE TEST_GROUP=None)

    target_check_style(testing_game)
endif()
