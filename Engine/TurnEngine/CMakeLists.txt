cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

set(TURNENGINE_SOURCES TurnEngine.cpp)

set(TURNENGINE_HEADERS TurnEngine.h)

if(WIN32)
else()
endif()

add_library(turnengine STATIC ${TURNENGINE_SOURCES} ${TURNENGINE_HEADERS})
target_check_style(turnengine)
