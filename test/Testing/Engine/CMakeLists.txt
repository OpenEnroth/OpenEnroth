cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

if(ENABLE_TESTS)
    set(TEST_ENGINE_SOURCES TestEventLoop.cpp
                            TestOpenGLContext.cpp
                            TestPlatform.cpp
                            TestWindow.cpp
                            GameWrapper.cpp)
    set(TEST_ENGINE_HEADERS TestEventLoop.h
                            TestOpenGLContext.h
                            TestPlatform.h
                            TestState.h
                            TestStateHandle.h
                            TestWindow.h
                            GameWrapper.h)

    add_library(test_engine ${TEST_ENGINE_SOURCES} ${TEST_ENGINE_HEADERS})
    target_link_libraries(test_engine platform utility)

    target_check_style(test_engine)
endif()
