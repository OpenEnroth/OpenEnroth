cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

set(GAME_SOURCES Application/Game.cpp
                 Application/GameBootstrapper.cpp
                 Application/GameConfig.cpp
                 Application/GameFactory.cpp
                 Application/GameMenu.cpp
                 Application/GameOver.cpp
                 Application/GameWindowHandler.cpp
                 Application/IocContainer.cpp
                 tools/MemBuffer.cpp)

set(GAME_HEADERS Application/Game.h
                 Application/GameConfig.h
                 Application/GameFactory.h
                 Application/GameMenu.h
                 Application/GameOver.h
                 Application/IocContainer.h
                 Application/GameWindowHandler.h
                 tools/MemBuffer.h)

if(WIN32)
else()
    set(CMAKE_CXX_FLAGS "-fpermissive ${CMAKE_CXX_FLAGS}")
endif()

add_library(game STATIC ${GAME_SOURCES} ${GAME_HEADERS})
target_check_style(game)

target_link_libraries(game engine platform graphics)
