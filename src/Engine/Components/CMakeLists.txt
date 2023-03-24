cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(ENGINE_COMPONENTS_SOURCES
        EngineControlComponent.cpp
        EngineController.cpp
        EngineDeterministicComponent.cpp
        EngineTraceComponent.cpp
        EngineTracer.cpp)

set(ENGINE_COMPONENTS_HEADERS
        EngineControlComponent.h
        EngineControlState.h
        EngineControlStateHandle.h
        EngineController.h
        EngineDeterministicComponent.h
        EngineTraceComponent.h
        EngineTracePlaybackFlags.h
        EngineTracer.h)

add_library(engine_components STATIC ${ENGINE_COMPONENTS_SOURCES} ${ENGINE_COMPONENTS_HEADERS})
target_check_style(engine_components)

target_link_libraries(engine_components engine platform utility library_application library_trace)
