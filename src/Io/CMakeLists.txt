cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(IO_SOURCES
        InputAction.cpp
        KeyboardActionMapping.cpp
        KeyboardInputHandler.cpp
        Mouse.cpp)

set(IO_HEADERS
        IKeyboardController.h
        InputAction.h
        Key.cpp
        Key.h
        KeyboardActionMapping.h
        KeyboardInputHandler.h
        Mouse.h)

add_library(io STATIC ${IO_SOURCES} ${IO_HEADERS})
target_link_libraries(io PUBLIC library_serialization PRIVATE SDL2::SDL2OE)
target_check_style(io)
