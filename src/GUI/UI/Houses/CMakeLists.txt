cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(GUI_UI_HOUSES_SOURCES
        MagicGuild.cpp)

set(GUI_UI_HOUSES_HEADERS
        MagicGuild.h)

add_library(gui_ui_houses STATIC ${GUI_UI_HOUSES_SOURCES} ${GUI_UI_HOUSES_HEADERS})
target_check_style(gui_ui_houses)

target_link_libraries(gui_ui_houses
                      utility)
