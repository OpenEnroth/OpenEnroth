cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(GUI_SOURCES GUIButton.cpp
                GUIFont.cpp
                GUIProgressBar.cpp
                GUIWindow.cpp
                GUIEnums.cpp
                UI/Spellbook.cpp
                UI/UIArena.cpp
                UI/UIBooks.cpp
                UI/UICharacter.cpp
                UI/UIChest.cpp
                UI/UICredits.cpp
                UI/UIDialogue.cpp
                UI/UIGame.cpp
                UI/UIGuilds.cpp
                UI/UIInventory.cpp
                UI/UIHouses.cpp
                UI/UIMainMenu.cpp
                UI/UIGameOver.cpp
                UI/UIPartyCreation.cpp
                UI/UIPopup.cpp
                UI/UIQuickReference.cpp
                UI/UIRest.cpp
                UI/UISaveLoad.cpp
                UI/UIShops.cpp
                UI/UISpell.cpp
                UI/UIStatusBar.cpp
                UI/UITransition.cpp
                UI/Books/AutonotesBook.cpp
                UI/Books/CalendarBook.cpp
                UI/Books/JournalBook.cpp
                UI/Books/LloydsBook.cpp
                UI/Books/MapBook.cpp
                UI/Books/QuestBook.cpp
                UI/Books/TownPortalBook.cpp
)

set(GUI_HEADERS GUIButton.h
                GUIDialogues.h
                GUIFont.h
                GUIProgressBar.h
                GUIWindow.h
                GUIEnums.h
                UI/Spellbook.h
                UI/UIArena.h
                UI/UIBooks.h
                UI/UICharacter.h
                UI/UIChest.h
                UI/UICredits.h
                UI/UIDialogue.h
                UI/UIGame.h
                UI/UIGuilds.h
                UI/UIInventory.h
                UI/UIHouses.h
                UI/UIHouseEnums.h
                UI/UIMainMenu.h
                UI/UIGameOver.h
                UI/UIPartyCreation.h
                UI/UIPopup.h
                UI/UIQuickReference.h
                UI/UIRest.h
                UI/UISaveLoad.h
                UI/UIShops.h
                UI/UISpell.h
                UI/UIStatusBar.h
                UI/UITransition.h
                UI/Books/AutonotesBook.h
                UI/Books/CalendarBook.h
                UI/Books/JournalBook.h
                UI/Books/LloydsBook.h
                UI/Books/MapBook.h
                UI/Books/QuestBook.h
                UI/Books/TownPortalBook.h
)

add_library(gui STATIC ${GUI_SOURCES} ${GUI_HEADERS})
target_check_style(gui)

target_link_libraries(gui arcomage game spells utility)
