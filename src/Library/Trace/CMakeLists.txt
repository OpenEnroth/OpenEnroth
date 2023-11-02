cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(LIBRARY_TRACE_SOURCES
        EventTrace.cpp)

set(LIBRARY_TRACE_HEADERS
        EventTrace.h
        PaintEvent.h)

add_library(library_trace STATIC ${LIBRARY_TRACE_SOURCES} ${LIBRARY_TRACE_HEADERS})
target_link_libraries(library_trace PUBLIC library_serialization library_platform_interface library_json)
target_check_style(library_trace)
