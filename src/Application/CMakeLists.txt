cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(APPLICATION_SOURCES
        Game.cpp
        GameConfig.cpp
        GameFactory.cpp
        GameKeyboardController.cpp
        GameMenu.cpp
        GameOptions.cpp
        GameOver.cpp
        GamePathResolver.cpp
        GameTraceHandler.cpp
        GameWindowHandler.cpp
        IocContainer.cpp)

set(APPLICATION_HEADERS
        Game.h
        GameConfig.h
        GameFactory.h
        GameKeyboardController.h
        GameMenu.h
        GameOptions.h
        GameOver.h
        GamePathResolver.h
        GameTraceHandler.h
        GameWindowHandler.h
        IocContainer.h)

add_library(application STATIC ${APPLICATION_SOURCES} ${APPLICATION_HEADERS})
target_check_style(application)

target_link_libraries(application
        CLI11::CLI11
        engine
        engine_components
        engine_graphics
        gui
        io
        library_config
        library_lod
        media
        platform
        utility)
