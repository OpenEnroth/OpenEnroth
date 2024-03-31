cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_ENVIRONMENT_INTERFACE_SOURCES)

set(LIBRARY_ENVIRONMENT_INTERFACE_HEADERS
        Environment.h
        EnvironmentEnums.h)

add_library(library_environment_interface INTERFACE ${LIBRARY_ENVIRONMENT_INTERFACE_SOURCES} ${LIBRARY_ENVIRONMENT_INTERFACE_HEADERS})
target_check_style(library_environment_interface)
