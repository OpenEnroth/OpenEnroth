cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(LIBRARY_PLATFORM_APPLICATION_SOURCES
        PlatformApplication.cpp)

set(LIBRARY_PLATFORM_APPLICATION_HEADERS
        PlatformApplication.h
        PlatformApplicationAware.h
        PlatformIntrospection.h)

add_library(library_platform_application STATIC ${LIBRARY_PLATFORM_APPLICATION_SOURCES} ${LIBRARY_PLATFORM_APPLICATION_HEADERS})
target_link_libraries(library_platform_application PUBLIC library_platform_interface library_platform_filters library_platform_proxy)
target_check_style(library_platform_application)
