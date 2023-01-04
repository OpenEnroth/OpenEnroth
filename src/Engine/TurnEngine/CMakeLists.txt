cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(TURNENGINE_SOURCES TurnEngine.cpp)

set(TURNENGINE_HEADERS TurnEngine.h)

add_library(turnengine STATIC ${TURNENGINE_SOURCES} ${TURNENGINE_HEADERS})
target_check_style(turnengine)
