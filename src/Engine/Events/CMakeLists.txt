cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(ENGINE_EVENTS_SOURCES
        EventEnums.cpp
        EventIR.cpp
        EventMap.cpp
        EventInterpreter.cpp
        Processor.cpp)

set(ENGINE_EVENTS_HEADERS
        EventIR.h
        EventMap.h
        EventInterpreter.h
        EventEnums.h
        Processor.h
        EventEnumFunctions.h)

add_library(engine_events STATIC ${ENGINE_EVENTS_SOURCES} ${ENGINE_EVENTS_HEADERS})
target_link_libraries(engine_events PUBLIC engine tl::generator)
target_check_style(engine_events)
