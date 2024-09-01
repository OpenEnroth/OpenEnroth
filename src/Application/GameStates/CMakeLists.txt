cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(APPLICATION_GAME_STATES_SOURCES
        CreditsState.cpp
        GameFsmBuilder.cpp
        LoadStep2State.cpp
        MainMenuState.cpp
        StartState.cpp
        VideoState.cpp)

set(APPLICATION_GAME_STATES_HEADERS
        CreditsState.h
        GameFsmBuilder.h
        LoadStep2State.h
        MainMenuState.h
        StartState.h
        VideoState.h)

add_library(application_game_states STATIC ${APPLICATION_GAME_STATES_SOURCES} ${APPLICATION_GAME_STATES_HEADERS})
target_check_style(application_game_states)

target_link_libraries(application_game_states
        PUBLIC
        library_fsm
        media
        utility)
