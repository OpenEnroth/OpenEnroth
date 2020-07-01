cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(GAME_SOURCES
    Game.cpp
    GameBootstrapper.cpp
    GameConfig.cpp
    GameFactory.cpp
    GameMenu.cpp
    GameOver.cpp
    GameState.cpp
    GameWindowHandler.cpp
    IocContainer.cpp
)

set(GAME_HEADERS
    Game.h
    GameConfig.h
    GameFactory.h
    GameMenu.h
    GameOver.h
    GameState.h
    GameWindowHandler.h
    IocContainer.h
)

if(WIN32)
else()
    set(CMAKE_CXX_FLAGS "-fpermissive ${CMAKE_CXX_FLAGS}")
endif()

add_library(game STATIC ${GAME_SOURCES} ${GAME_HEADERS})
target_check_style(game)

target_link_libraries(game gameui engine platform graphics)

add_subdirectory(Ui)
