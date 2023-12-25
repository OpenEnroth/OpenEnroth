cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(ENGINE_TURN_ENGINE_SOURCES
        TurnEngine.cpp)

set(ENGINE_TURN_ENGINE_HEADERS
        TurnEngine.h
        TurnEngineEnums.h)

add_library(engine_turn_engine STATIC ${ENGINE_TURN_ENGINE_SOURCES} ${ENGINE_TURN_ENGINE_HEADERS})
target_link_libraries(engine_turn_engine PUBLIC engine)
target_check_style(engine_turn_engine)
