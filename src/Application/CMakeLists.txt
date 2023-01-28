cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(GAME_SOURCES
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

set(GAME_HEADERS
    Game.h
    GameConfig.h
    GameFactory.h
    GameKeyboardController.h
    GameMenu.h
    GameOver.h
    GamePathResolver.h
    GameOptions.h
    GameTraceHandler.h
    GameWindowHandler.h
    IocContainer.h)

add_library(game STATIC ${GAME_SOURCES} ${GAME_HEADERS})
target_check_style(game)

target_link_libraries(game io gui engine platform graphics media lod utility config trace application CLI11::CLI11)
