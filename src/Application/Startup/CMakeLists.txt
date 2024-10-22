cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(APPLICATION_STARTUP_SOURCES
        PathResolver.cpp
        GameStarter.cpp
        LogStarter.cpp
        FileSystemStarter.cpp)

set(APPLICATION_STARTUP_HEADERS
        PathResolver.h
        GameStarter.h
        GameStarterOptions.h
        LogStarter.h
        FileSystemStarter.h)

add_library(application_startup STATIC ${APPLICATION_STARTUP_SOURCES} ${APPLICATION_STARTUP_HEADERS})
target_check_style(application_startup)

target_link_libraries(application_startup
        PUBLIC
        engine
        engine_components_control
        engine_components_deterministic
        engine_components_trace
        engine_graphics
        engine_graphics_renderer
        gui
        library_config
        library_fsm
        library_platform_null
        library_platform_implementation
        library_platform_application
        library_environment_implementation
        library_logger
        scripting
        utility)

