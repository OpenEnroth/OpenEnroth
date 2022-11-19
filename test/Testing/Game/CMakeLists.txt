cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

if(ENABLE_TESTS)
    set(GAME_TEST_SOURCES GameTest.cpp
                          GameTestEventLoop.cpp
                          GameTestMain.cpp
                          GameTestOpenGLContext.cpp
                          GameTestPlatform.cpp
                          GameTestWindow.cpp
                          GameWrapper.cpp)
    set(GAME_TEST_HEADERS GameTest.h
                          GameTestEventLoop.h
                          GameTestOpenGLContext.h
                          GameTestPlatform.h
                          GameTestState.h
                          GameTestStateHandle.h
                          GameTestWindow.h
                          GameWrapper.h)

    add_executable(World_of_Might_and_Magic_GameTest ${GAME_TEST_SOURCES} ${GAME_TEST_HEADERS})
    target_fix_libcxx_assertions(World_of_Might_and_Magic_GameTest)
    target_link_libraries(World_of_Might_and_Magic_GameTest game GTest::gtest)
    target_compile_definitions(World_of_Might_and_Magic_GameTest PRIVATE TEST_GROUP=GameTest)

    target_check_style(World_of_Might_and_Magic_GameTest)
endif()
