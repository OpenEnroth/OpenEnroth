cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(APPLICATION_SOURCES
        Game.cpp
        GameConfig.cpp
        GameKeyboardController.cpp
        GameMenu.cpp
        GameOver.cpp
        GamePathResolver.cpp
        GameStarter.cpp
        GameTraceHandler.cpp
        GameWindowHandler.cpp
        GameIocContainer.cpp)

set(APPLICATION_HEADERS
        Game.h
        GameConfig.h
        GameKeyboardController.h
        GameMenu.h
        GameOver.h
        GamePathResolver.h
        GameStarter.h
        GameStarterOptions.h
        GameTraceHandler.h
        GameWindowHandler.h
        GameIocContainer.h)

add_library(application STATIC ${APPLICATION_SOURCES} ${APPLICATION_HEADERS})
target_check_style(application)

target_link_libraries(application
        PUBLIC
        engine
        engine_components_control
        engine_components_deterministic
        engine_components_trace
        engine_graphics
        engine_graphics_renderer
        game_states
        gui
        io
        library_config
        library_fsm
        library_lod
        media
        library_platform_null
        library_platform_implementation
        library_environment_implementation
        scripting
        utility
        imgui)

add_subdirectory(GameStates)
