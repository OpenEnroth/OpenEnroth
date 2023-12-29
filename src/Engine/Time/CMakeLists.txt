cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(ENGINE_TIME_SOURCES
        Timer.cpp)

set(ENGINE_TIME_HEADERS
        Timer.h
        Time.h
        Duration.h)

add_library(engine_time STATIC ${ENGINE_TIME_SOURCES} ${ENGINE_TIME_HEADERS})
target_link_libraries(engine_time PUBLIC engine)
target_check_style(engine_time)
