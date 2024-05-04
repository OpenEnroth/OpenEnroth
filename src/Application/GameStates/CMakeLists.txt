cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(GAME_STATES_SOURCES
        CreditsState.cpp
        GameFSMBuilder.cpp
        LoadSaveSlotState.cpp
        LoadStep2State.cpp
        MainMenuState.cpp
        VideoState.cpp)

set(GAME_STATES_HEADERS
        CreditsState.h
        GameFSMBuilder.h
        LoadSaveSlotState.h
        LoadStep2State.h
        MainMenuState.h
        VideoState.h)

add_library(game_states STATIC ${GAME_STATES_SOURCES} ${GAME_STATES_HEADERS})
target_check_style(game_states)

target_link_libraries(game_states
        PUBLIC
        library_fsm
        media
        utility)
