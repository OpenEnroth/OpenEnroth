#pragma once

#include <string>

#include "GUI/GUIWindow.h"

enum class BookButtonAction {
    BOOK_ZOOM_IN = 0,
    BOOK_ZOOM_OUT = 1,
    BOOK_SCROLL_UP = 2,
    BOOK_SCROLL_DOWN = 3,
    BOOK_SCROLL_RIGHT = 4,
    BOOK_SCROLL_LEFT = 5,
    BOOK_NOTES_POTION = 6,
    BOOK_NOTES_FOUNTAIN = 7,
    BOOK_NOTES_OBELISK = 8,
    BOOK_NOTES_SEER = 9,
    BOOK_NOTES_MISC = 10,
    BOOK_NOTES_INSTRUCTORS = 11,
    BOOK_NEXT_PAGE = 12,
    BOOK_PREV_PAGE = 13
};
using enum BookButtonAction;

class GUIWindow_Book : public GUIWindow {
 public:
    GUIWindow_Book();
    virtual ~GUIWindow_Book() {}

    virtual void Release();

 protected:
    GUIWindow *pChildBooksOverlay{ nullptr };

 private:
    void InitializeFonts();
};


class GUIWindow_BooksButtonOverlay : public GUIWindow {
 public:
    GUIWindow_BooksButtonOverlay(Pointi position, Sizei dimensions, GUIButton *button, const std::string &hint = std::string()) :
        GUIWindow(WINDOW_BooksButtonOverlay, position, dimensions, button, hint)
    {}
    virtual ~GUIWindow_BooksButtonOverlay() {}

    virtual void Update();
};

class GraphicsImage;
extern GraphicsImage *ui_book_button8_off;
extern GraphicsImage *ui_book_button8_on;
extern GraphicsImage *ui_book_button7_off;
extern GraphicsImage *ui_book_button7_on;
extern GraphicsImage *ui_book_button6_off;
extern GraphicsImage *ui_book_button6_on;
extern GraphicsImage *ui_book_button5_off;
extern GraphicsImage *ui_book_button5_on;
extern GraphicsImage *ui_book_button4_off;
extern GraphicsImage *ui_book_button4_on;
extern GraphicsImage *ui_book_button3_off;
extern GraphicsImage *ui_book_button3_on;
extern GraphicsImage *ui_book_button2_off;
extern GraphicsImage *ui_book_button2_on;
extern GraphicsImage *ui_book_button1_off;
extern GraphicsImage *ui_book_button1_on;

extern GraphicsImage *ui_book_map_frame;
extern GraphicsImage *ui_book_quest_div_bar;

extern bool bookButtonClicked;
extern BookButtonAction bookButtonAction;
