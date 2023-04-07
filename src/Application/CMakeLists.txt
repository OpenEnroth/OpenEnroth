cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(APPLICATION_SOURCES
        Game.cpp
        GameConfig.cpp
        GameKeyboardController.cpp
        GameMenu.cpp
        GameOptions.cpp
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
        GameOptions.h
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
        CLI11::CLI11
        engine
        engine_components_control
        engine_components_deterministic
        engine_components_trace
        engine_graphics
        gui
        io
        library_config
        library_lod
        media
        platform
        utility)
