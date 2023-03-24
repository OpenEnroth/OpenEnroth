cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(ENGINE_TURNENGINE_SOURCES
        TurnEngine.cpp)

set(ENGINE_TURNENGINE_HEADERS
        TurnEngine.h)

add_library(engine_turnengine STATIC ${ENGINE_TURNENGINE_SOURCES} ${ENGINE_TURNENGINE_HEADERS})
target_link_libraries(engine_turnengine engine)
target_check_style(engine_turnengine)
