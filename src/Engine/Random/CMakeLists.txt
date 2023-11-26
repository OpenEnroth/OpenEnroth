cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(ENGINE_RANDOM_SOURCES
        Random.cpp
        RandomEnums.cpp)

set(ENGINE_RANDOM_HEADERS
        Random.h
        RandomEnums.h)

add_library(engine_random STATIC ${ENGINE_RANDOM_SOURCES} ${ENGINE_RANDOM_HEADERS})
target_link_libraries(engine_random PUBLIC library_random library_serialization)
target_check_style(engine_random)
