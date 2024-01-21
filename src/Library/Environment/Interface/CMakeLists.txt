cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(LIBRARY_ENVIRONMENT_INTERFACE_SOURCES
        Environment.cpp)

set(LIBRARY_ENVIRONMENT_INTERFACE_HEADERS
        Environment.h
        EnvironmentEnums.h
        PathResolutionConfig.h
        utils.h)

add_library(library_environment_interface INTERFACE ${LIBRARY_ENVIRONMENT_INTERFACE_HEADERS} ${LIBRARY_ENVIRONMENT_INTERFACE_SOURCES})
target_check_style(library_environment_interface)
