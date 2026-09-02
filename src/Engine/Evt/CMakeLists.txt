cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(ENGINE_EVENTS_SOURCES
        EvtEnums.cpp
        EvtInstruction.cpp
        EvtProgram.cpp
        EvtInterpreter.cpp
        Processor.cpp)

set(ENGINE_EVENTS_HEADERS
        EvtInstruction.h
        EvtProgram.h
        EvtInterpreter.h
        EvtEnums.h
        Processor.h
        EvtEnumFunctions.h)

add_library(engine_events STATIC ${ENGINE_EVENTS_SOURCES} ${ENGINE_EVENTS_HEADERS})
target_link_libraries(engine_events PUBLIC engine tl::generator)
target_check_style(engine_events)
