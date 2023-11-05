cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(LIBRARY_PLATFORM_WIN_SOURCES
        WinPlatform.cpp)

set(LIBRARY_PLATFORM_WIN_HEADERS
        WinPlatform.h)

if(BUILD_PLATFORM STREQUAL "windows")
    add_library(library_platform_win STATIC ${LIBRARY_PLATFORM_WIN_SOURCES} ${LIBRARY_PLATFORM_WIN_HEADERS})
    target_check_style(library_platform_win)
    target_link_libraries(library_platform_win PUBLIC library_platform_sdl)

    add_library(library_platform_implementation INTERFACE)
    target_link_libraries(library_platform_implementation INTERFACE library_platform_win)
endif()
