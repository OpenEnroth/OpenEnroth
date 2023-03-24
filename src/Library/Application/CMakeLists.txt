cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(LIBRARY_APPLICATION_SOURCES
        PlatformApplication.cpp)

set(LIBRARY_APPLICATION_HEADERS
        PlatformApplication.h
        PlatformApplicationAware.h
        PlatformIntrospection.h)

add_library(library_application STATIC ${LIBRARY_APPLICATION_SOURCES} ${LIBRARY_APPLICATION_HEADERS})
target_link_libraries(library_application platform)
target_check_style(library_application)
