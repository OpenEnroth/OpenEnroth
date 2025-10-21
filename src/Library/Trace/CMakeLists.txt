cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_TRACE_SOURCES
        EventTrace.cpp
        EventTraceMigrations.cpp)

set(LIBRARY_TRACE_HEADERS
        EventTrace.h
        EventTraceMigrations.h
        PaintEvent.h)

add_library(library_trace STATIC ${LIBRARY_TRACE_SOURCES} ${LIBRARY_TRACE_HEADERS})
target_check_style(library_trace)
target_link_libraries(library_trace PUBLIC
        library_serialization
        library_json
        library_platform_interface
        library_config
        library_geometry)
