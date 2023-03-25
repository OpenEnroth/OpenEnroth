cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(LIBRARY_TRACE_SOURCES
        EventTrace.cpp)

set(LIBRARY_TRACE_HEADERS
        EventTrace.h
        PaintEvent.h)

add_library(library_trace STATIC ${LIBRARY_TRACE_SOURCES} ${LIBRARY_TRACE_HEADERS})
target_link_libraries(library_trace library_serialization platform library_json)
target_check_style(library_trace)
