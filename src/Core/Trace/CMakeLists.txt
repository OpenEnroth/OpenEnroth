cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(CORE_TRACE_SOURCES
        EventTrace.cpp
        EventTraceMigrations.cpp)

set(CORE_TRACE_HEADERS
        EventTrace.h
        EventTraceMigrations.h
        PaintEvent.h)

add_library(core_trace STATIC ${CORE_TRACE_SOURCES} ${CORE_TRACE_HEADERS})
target_check_style(core_trace)
target_link_libraries(core_trace PUBLIC
        core_serialization
        library_serialization
        library_json
        library_platform_interface
        library_config
        library_geometry)
