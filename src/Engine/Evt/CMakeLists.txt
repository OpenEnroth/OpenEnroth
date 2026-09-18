cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(ENGINE_EVENTS_SOURCES
        EvtCommands.cpp
        EvtEnums.cpp
        EvtInstruction.cpp
        EvtProgram.cpp
        EvtVariables.cpp
        EvtInterpreter.cpp
        Processor.cpp)

set(ENGINE_EVENTS_HEADERS
        EvtCommands.h
        EvtInstruction.h
        EvtProgram.h
        EvtVariables.h
        EvtInterpreter.h
        EvtEnums.h
        Processor.h
        EvtEnumFunctions.h)

add_library(engine_events STATIC ${ENGINE_EVENTS_SOURCES} ${ENGINE_EVENTS_HEADERS})
target_link_libraries(engine_events PUBLIC engine tl::generator)
target_check_style(engine_events)

if(OE_BUILD_TESTS)
    set(TEST_ENGINE_EVENTS_SOURCES
            Tests/EvtCommands_ut.cpp)
    add_library(test_engine_events OBJECT ${TEST_ENGINE_EVENTS_SOURCES})
    target_link_libraries(test_engine_events PUBLIC testing_unit engine_events)
    target_check_style(test_engine_events)
    target_link_libraries(OpenEnroth_GameTest PUBLIC test_engine_events)
endif()
