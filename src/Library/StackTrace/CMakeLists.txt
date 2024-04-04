cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_STACK_TRACE_SOURCES
        StackTrace.cpp
        StackTraceOnCrash.cpp)

set(LIBRARY_STACK_TRACE_HEADERS
        StackTrace.h
        StackTraceOnCrash.h)

add_library(library_stack_trace STATIC ${LIBRARY_STACK_TRACE_SOURCES} ${LIBRARY_STACK_TRACE_HEADERS})
target_link_libraries(library_stack_trace PUBLIC utility)
target_check_style(library_stack_trace)

# backward-cpp doesn't support Android...
if (NOT OE_BUILD_PLATFORM STREQUAL "android")
    target_link_libraries(library_stack_trace PRIVATE Backward::Backward)
endif ()
