cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(IO_SOURCES InputAction.cpp
               KeyboardActionMapping.cpp
               KeyboardInputHandler.cpp
               Mouse.cpp)

set(IO_HEADERS IKeyboardController.h
               InputAction.h
               KeyboardActionMapping.h
               KeyboardInputHandler.h
               Mouse.h)

add_library(io STATIC ${IO_SOURCES} ${IO_HEADERS})
target_check_style(io)
