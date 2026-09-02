cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_PLATFORM_NULL_SOURCES
        NullEventLoop.cpp
        NullOpenGLContext.cpp
        NullPlatform.cpp
        NullWindow.cpp)

set(LIBRARY_PLATFORM_NULL_HEADERS
        NullEventLoop.h
        NullOpenGLContext.h
        NullPlatform.h
        NullPlatformOptions.h
        NullPlatformSharedState.h
        NullWindow.h)

add_library(library_platform_null STATIC ${LIBRARY_PLATFORM_NULL_SOURCES} ${LIBRARY_PLATFORM_NULL_HEADERS})
target_check_style(library_platform_null)
target_link_libraries(library_platform_null PUBLIC utility library_platform_interface)
