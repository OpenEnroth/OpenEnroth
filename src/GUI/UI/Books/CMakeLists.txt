cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(GUI_UI_BOOKS_SOURCES
        AutonotesBook.cpp
        CalendarBook.cpp
        JournalBook.cpp
        LloydsBook.cpp
        MapBook.cpp
        QuestBook.cpp
        TownPortalBook.cpp)

set(GUI_UI_BOOKS_HEADERS
        AutonotesBook.h
        CalendarBook.h
        JournalBook.h
        LloydsBook.h
        MapBook.h
        QuestBook.h
        TownPortalBook.h)

add_library(gui_ui_books STATIC ${GUI_UI_BOOKS_SOURCES} ${GUI_UI_BOOKS_HEADERS})
target_check_style(gui_ui_books)

target_link_libraries(gui_ui_books PUBLIC utility)
