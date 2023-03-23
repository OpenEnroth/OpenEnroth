cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(COMPONENTS_SOURCES EngineController.cpp
                       EngineControlComponent.cpp
                       EngineDeterministicComponent.cpp
                       EngineTraceComponent.cpp
                       EngineTracer.cpp)

set(COMPONENTS_HEADERS EngineController.h
                       EngineControlComponent.h
                       EngineControlState.h
                       EngineControlStateHandle.h
                       EngineDeterministicComponent.h
                       EngineTracePlaybackFlags.h
                       EngineTraceComponent.h
                       EngineTracer.h)

# TODO(captainurist): lib naming policy needs revisiting. engine_components would make more sense here.
add_library(components STATIC ${COMPONENTS_SOURCES} ${COMPONENTS_HEADERS})
target_check_style(components)

target_link_libraries(components engine platform utility application)
