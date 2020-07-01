cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(GAME_UI_SOURCES
    ImGui_Data.cpp
    ImGui_ImplOpenGL2.cpp
    ImGui_ImplSDL2.cpp
    Ui.cpp
)

set(GAME_UI_HEADERS
    ImGui_Data.h
    ImGui_ImplOpenGL2.h
    ImGui_ImplSDL2.h
    Ui.h
)

if(WIN32)
else()
    set(CMAKE_CXX_FLAGS "-fpermissive ${CMAKE_CXX_FLAGS}")
endif()

add_library(gameui STATIC ${GAME_UI_SOURCES} ${GAME_UI_SOURCES})
target_link_libraries(gameui imgui)

target_check_style(gameui)
