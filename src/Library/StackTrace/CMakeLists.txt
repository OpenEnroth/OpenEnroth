cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_STACK_TRACE_SOURCES
        StackTrace.cpp
        StackTraceOnCrash.cpp)

set(LIBRARY_STACK_TRACE_HEADERS
        StackTrace.h
        StackTraceOnCrash.h)

add_library(library_stack_trace STATIC ${LIBRARY_STACK_TRACE_SOURCES} ${LIBRARY_STACK_TRACE_HEADERS})
target_link_libraries(library_stack_trace PUBLIC utility)
target_check_cpp_style(library_stack_trace)

# cpptrace doesn't support Android...
if (NOT OE_BUILD_PLATFORM STREQUAL "android")
    target_link_libraries(library_stack_trace PRIVATE cpptrace::cpptrace)
endif ()

# The crash handler walks the stack with StackWalk64.
if (OE_BUILD_PLATFORM STREQUAL "windows")
    target_link_libraries(library_stack_trace PRIVATE dbghelp)
endif ()

if(OE_BUILD_TESTS)
    set(TEST_LIBRARY_STACK_TRACE_SOURCES
            Tests/StackTrace_ut.cpp)

    add_library(test_library_stack_trace OBJECT ${TEST_LIBRARY_STACK_TRACE_SOURCES})
    target_link_libraries(test_library_stack_trace PUBLIC testing_unit library_stack_trace)

    target_check_cpp_style(test_library_stack_trace)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_library_stack_trace)
endif()
