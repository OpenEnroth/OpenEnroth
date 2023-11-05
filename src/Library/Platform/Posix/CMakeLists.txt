cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(LIBRARY_PLATFORM_POSIX_SOURCES
        PosixPlatform.cpp)

set(LIBRARY_PLATFORM_POSIX_HEADERS)

if(NOT BUILD_PLATFORM STREQUAL "windows")
    add_library(library_platform_posix STATIC ${LIBRARY_PLATFORM_POSIX_SOURCES} ${LIBRARY_PLATFORM_POSIX_HEADERS})
    target_check_style(library_platform_posix)
    target_link_libraries(library_platform_posix PUBLIC library_platform_sdl)

    add_library(library_platform_implementation INTERFACE)
    target_link_libraries(library_platform_implementation INTERFACE library_platform_posix)
endif()
