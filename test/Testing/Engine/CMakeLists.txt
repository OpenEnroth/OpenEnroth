cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

if(ENABLE_TESTS)
    set(TEST_ENGINE_SOURCES TestEventLoop.cpp
                            TestGamepad.cpp
                            TestOpenGLContext.cpp
                            TestPlatform.cpp
                            TestWindow.cpp)
    set(TEST_ENGINE_HEADERS TestEventLoop.h
                            TestGamepad.h
                            TestOpenGLContext.h
                            TestPlatform.h
                            TestState.h
                            TestStateHandle.h
                            TestWindow.h)

    add_library(test_engine ${TEST_ENGINE_SOURCES} ${TEST_ENGINE_HEADERS})
    target_link_libraries(test_engine platform utility)

    target_check_style(test_engine)
endif()
