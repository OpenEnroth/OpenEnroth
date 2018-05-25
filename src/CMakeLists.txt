cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(GAME_SOURCES Application/Configuration.cpp
                 Application/Game.cpp
                 Application/GameFactory.cpp
                 Application/GameMenu.cpp
                 Application/GameOver.cpp
                 Application/IocContainer.cpp
                 Application/GameWindowHandler.cpp
                 tools/MemBuffer.cpp)

set(GAME_HEADERS Application/Configuration.h
                 Application/Game.h
                 Application/GameFactory.h
                 Application/GameMenu.h
                 Application/GameOver.h
                 Application/IocContainer.h
                 Application/GameWindowHandler.h
                 tools/MemBuffer.h)

if(WIN32)
else()
endif()

add_library(game STATIC ${GAME_SOURCES} ${GAME_HEADERS})
target_check_style(game)
