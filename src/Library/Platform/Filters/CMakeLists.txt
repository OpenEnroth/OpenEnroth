cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(LIBRARY_PLATFORM_FILTERS_SOURCES
        FilteringEventHandler.cpp
        PlatformEventFilter.cpp)

set(LIBRARY_PLATFORM_FILTERS_HEADERS
        FilteringEventHandler.h
        PlatformEventFilter.h)

add_library(library_platform_filters STATIC ${LIBRARY_PLATFORM_FILTERS_SOURCES} ${LIBRARY_PLATFORM_FILTERS_HEADERS})
target_check_style(library_platform_filters)
target_link_libraries(library_platform_filters PUBLIC utility library_platform_interface)
