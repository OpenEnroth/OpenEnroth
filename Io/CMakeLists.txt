cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

set(IO_SOURCES
    GameKey.cpp
    InputAction.cpp
    KeyboardActionMapping.cpp
    KeyboardInputHandler.cpp
    Mouse.cpp
)

set(IO_HEADERS
    GameKey.h
    IKeyboardController.h
    IMouseController.h
    InputAction.h
    KeyboardActionMapping.h
    KeyboardInputHandler.h
    Mouse.h
)

add_library(io STATIC ${IO_SOURCES} ${IO_HEADERS})
target_check_style(io)
