cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_ENVIRONMENT_WIN_SOURCES
        WinEnvironment.cpp)

set(LIBRARY_ENVIRONMENT_WIN_HEADERS
        WinEnvironment.h)

if(OE_BUILD_PLATFORM STREQUAL "windows")
    add_library(library_environment_win STATIC ${LIBRARY_ENVIRONMENT_WIN_SOURCES} ${LIBRARY_ENVIRONMENT_WIN_HEADERS})
    target_check_style(library_environment_win)
    target_link_libraries(library_environment_win PUBLIC library_environment_interface utility)

    add_library(library_environment_implementation INTERFACE)
    target_link_libraries(library_environment_implementation INTERFACE library_environment_win)
endif()
