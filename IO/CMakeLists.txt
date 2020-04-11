cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(IO_SOURCES
    GameKey.cpp
    InputAction.cpp
    Keyboard.cpp
    Mouse.cpp
    UserInputHandler.cpp
)

set(IO_HEADERS
    GameKey.h
    InputAction.h
    IUserInputProvider.h
    Keyboard.h
    KeyboardActionMapping.h
    Mouse.h
    UserInputHandler.h
)

add_library(io STATIC ${IO_SOURCES} ${IO_HEADERS})
target_check_style(io)
