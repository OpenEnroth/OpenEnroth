cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(IO_SOURCES
        InputEnumFunctions.cpp
        KeyboardActionMapping.cpp
        KeyboardInputHandler.cpp
        Mouse.cpp)

set(IO_HEADERS
        IKeyboardController.h
        InputEnums.h
        InputEnumFunctions.h
        KeyboardActionMapping.h
        KeyboardInputHandler.h
        Mouse.h)

add_library(io STATIC ${IO_SOURCES} ${IO_HEADERS})
target_link_libraries(io PUBLIC library_serialization)
target_check_style(io)
