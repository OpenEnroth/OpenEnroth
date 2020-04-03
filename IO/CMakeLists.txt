cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(IO_SOURCES Keyboard.cpp
               Mouse.cpp)

set(IO_HEADERS Keyboard.h
               Mouse.h)

add_library(io STATIC ${IO_SOURCES} ${IO_HEADERS})
target_check_style(io)
