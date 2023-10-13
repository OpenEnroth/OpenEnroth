cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

if(OE_BUILD_TESTS)
    set(TESTING_GAME_SOURCES
            CharacterTapeRecorder.cpp
            CommonTapeRecorder.cpp
            GameTest.cpp
            TestController.cpp)
    set(TESTING_GAME_HEADERS
            CharacterTapeRecorder.h
            CommonTapeRecorder.h
            GameTest.h
            TestTape.h
            TestController.h
            AccessibleVector.h)

    add_library(testing_game ${TESTING_GAME_SOURCES} ${TESTING_GAME_HEADERS})
    target_link_libraries(testing_game PUBLIC application testing_extensions GTest::gtest)

    target_check_style(testing_game)
endif()
