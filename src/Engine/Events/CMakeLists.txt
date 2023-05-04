cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(ENGINE_EVENTS_SOURCES
        EventIR.cpp
        EventMap.cpp
        EventInterpreter.cpp
        Loader.cpp
        Processor.cpp)

set(ENGINE_EVENTS_HEADERS
        EventIR.h
        EventMap.h
        EventInterpreter.h
        EventEnums.h
        Loader.h
        Processor.h)

add_library(engine_events STATIC ${ENGINE_EVENTS_SOURCES} ${ENGINE_EVENTS_HEADERS})
target_link_libraries(engine_events engine)
target_check_style(engine_events)
