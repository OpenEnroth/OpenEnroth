cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_ENVIRONMENT_POSIX_SOURCES
        PosixEnvironment.cpp)

set(LIBRARY_ENVIRONMENT_POSIX_HEADERS
        PosixEnvironment.h)

if(NOT OE_BUILD_PLATFORM STREQUAL "windows" AND NOT OE_BUILD_PLATFORM STREQUAL "android")
    add_library(library_environment_posix STATIC ${LIBRARY_ENVIRONMENT_POSIX_SOURCES} ${LIBRARY_ENVIRONMENT_POSIX_HEADERS})
    target_check_style(library_environment_posix)
    target_link_libraries(library_environment_posix PUBLIC library_environment_interface)

    add_library(library_environment_implementation INTERFACE)
    target_link_libraries(library_environment_implementation INTERFACE library_environment_posix)
endif()
