cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(ENGINE_COMPONENTS_CONTROL_SOURCES
        EngineControlComponent.cpp
        EngineController.cpp)

set(ENGINE_COMPONENTS_CONTROL_HEADERS
        EngineControlComponent.h
        EngineControlState.h
        EngineControlStateHandle.h
        EngineController.h)

add_library(engine_components_control STATIC ${ENGINE_COMPONENTS_CONTROL_SOURCES} ${ENGINE_COMPONENTS_CONTROL_HEADERS})
target_check_style(engine_components_control)

target_link_libraries(engine_components_control PUBLIC
        utility
        engine
        gui
        arcomage
        library_filesystem_memory
        library_platform_interface
        library_platform_application)
