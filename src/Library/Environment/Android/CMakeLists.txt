cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(LIBRARY_ENVIRONMENT_ANDROID_SOURCES
        AndroidEnvironment.cpp)

set(LIBRARY_ENVIRONMENT_ANDROID_HEADERS
        AndroidEnvironment.h)

if(BUILD_PLATFORM STREQUAL "android")
    add_library(library_environment_android STATIC ${LIBRARY_ENVIRONMENT_ANDROID_SOURCES} ${LIBRARY_ENVIRONMENT_ANDROID_HEADERS})
    target_check_style(library_environment_android)
    target_link_libraries(library_environment_android PUBLIC library_environment_interface PRIVATE SDL2::SDL2OE)

    add_library(library_environment_implementation INTERFACE)
    target_link_libraries(library_environment_implementation INTERFACE library_environment_android)
endif()
