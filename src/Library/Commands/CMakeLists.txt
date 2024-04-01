cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(LIBRARY_COMMANDS_SOURCES
        CommandManager.cpp)

set(LIBRARY_COMMANDS_HEADERS
        CommandManager.h
        Command.h)

add_library(library_commands STATIC ${LIBRARY_COMMANDS_SOURCES} ${LIBRARY_COMMANDS_HEADERS})
target_check_style(library_commands)
target_link_libraries(library_commands
        PUBLIC
        utility
        PRIVATE)