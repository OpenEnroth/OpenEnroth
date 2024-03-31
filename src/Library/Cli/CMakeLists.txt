cmake_minimum_required(VERSION 3.27)

set(LIBRARY_CLI_SOURCES
        CliApp.cpp)

set(LIBRARY_CLI_HEADERS
        CliApp.h)

add_library(library_cli ${LIBRARY_CLI_SOURCES} ${LIBRARY_CLI_HEADERS})
target_link_libraries(library_cli CLI11::CLI11)
target_check_style(library_cli)
