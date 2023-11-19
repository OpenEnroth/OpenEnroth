cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(ENGINE_RANDOM_SOURCES
        Random.cpp
        RandomEngineFactory.cpp
        RandomEnums.cpp
        TracingRandomEngine.cpp)

set(ENGINE_RANDOM_HEADERS
        Random.h
        RandomEngineFactory.h
        RandomEnums.h
        TracingRandomEngine.h)

add_library(engine_random STATIC ${ENGINE_RANDOM_SOURCES} ${ENGINE_RANDOM_HEADERS})
target_link_libraries(engine_random PUBLIC library_random library_serialization library_stack_trace)
target_check_style(engine_random)
