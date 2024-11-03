cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(APPLICATION_SOURCES
        Game.cpp
        GameConfig.cpp
        GameKeyboardController.cpp
        GameMenu.cpp
        GameOver.cpp
        GameTraceHandler.cpp
        GameWindowHandler.cpp)

set(APPLICATION_HEADERS
        Game.h
        GameConfig.h
        GameKeyboardController.h
        GameMenu.h
        GameOver.h
        GameTraceHandler.h
        GameWindowHandler.h)

add_library(application STATIC ${APPLICATION_SOURCES} ${APPLICATION_HEADERS})
target_check_style(application)

target_link_libraries(application
        PUBLIC
        engine
        engine_components_control
        engine_components_trace
        engine_graphics
        engine_graphics_renderer
        application_game_states
        application_startup
        gui
        io
        library_config
        library_fsm
        library_lod
        media
        library_platform_application
        library_platform_interface
        scripting
        utility
        imgui)

add_subdirectory(GameStates)
add_subdirectory(Startup)
