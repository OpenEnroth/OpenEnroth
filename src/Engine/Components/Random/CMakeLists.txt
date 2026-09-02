cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(ENGINE_COMPONENTS_RANDOM_SOURCES
        EngineRandomComponent.cpp
        TracingRandomEngine.cpp)

set(ENGINE_COMPONENTS_RANDOM_HEADERS
        EngineRandomComponent.h
        TracingRandomEngine.h)

add_library(engine_components_random STATIC ${ENGINE_COMPONENTS_RANDOM_SOURCES} ${ENGINE_COMPONENTS_RANDOM_HEADERS})
target_check_style(engine_components_random)

target_link_libraries(engine_components_random PUBLIC
        engine_random
        library_random
        library_stack_trace
        library_platform_application)
