cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(TRACE_SOURCES EventTrace.cpp
                  EventTracer.cpp)

set(TRACE_HEADERS EventTrace.h
                  EventTracer.h
                  PaintEvent.h)

add_library(trace STATIC ${TRACE_SOURCES} ${TRACE_HEADERS})
target_link_libraries(trace serialization platform json random)
target_check_style(trace)
