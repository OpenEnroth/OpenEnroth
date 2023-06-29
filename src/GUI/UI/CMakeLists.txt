cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(GUI_UI_SOURCES
        ItemGrid.cpp
        UISpellbook.cpp
        UIArena.cpp
        UIBooks.cpp
        UICharacter.cpp
        UIChest.cpp
        UICredits.cpp
        UIDialogue.cpp
        UIGame.cpp
        UIGameOver.cpp
        UIHouses.cpp
        UIInventory.cpp
        UIMainMenu.cpp
        UIPartyCreation.cpp
        UIPopup.cpp
        UIQuickReference.cpp
        UIRest.cpp
        UISaveLoad.cpp
        UISpell.cpp
        UIStatusBar.cpp
        UITransition.cpp
        UIMessageScroll.cpp)

set(GUI_UI_HEADERS
        ItemGrid.h
        UISpellbook.h
        UIArena.h
        UIBooks.h
        UICharacter.h
        UIChest.h
        UICredits.h
        UIDialogue.h
        UIGame.h
        UIGameOver.h
        UIHouseEnums.h
        UIHouses.h
        UIInventory.h
        UIMainMenu.h
        UIPartyCreation.h
        UIPopup.h
        UIQuickReference.h
        UIRest.h
        UISaveLoad.h
        UISpell.h
        UIStatusBar.h
        UITransition.h
        UIMessageScroll.h)

add_library(gui_ui STATIC ${GUI_UI_SOURCES} ${GUI_UI_HEADERS})
target_check_style(gui_ui)

target_link_libraries(gui_ui
                      gui_ui_books
                      gui_ui_houses
                      arcomage
                      engine_spells
                      utility)

add_subdirectory(Books)
add_subdirectory(Houses)
