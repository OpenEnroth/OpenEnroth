#pragma once

#include <string>

#include "GUI/GUIWindow.h"

class GraphicsImage;

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

    /**
     * @offset 0x411597
     */
    virtual void Release() override;

    void bookButtonClicked(BookButtonAction action);

 protected:
    GUIWindow *pChildBooksOverlay{ nullptr };

    GraphicsImage *ui_book_button8_off{ nullptr };
    GraphicsImage *ui_book_button8_on{ nullptr };
    GraphicsImage *ui_book_button7_off{ nullptr };
    GraphicsImage *ui_book_button7_on{ nullptr };
    GraphicsImage *ui_book_button6_off{ nullptr };
    GraphicsImage *ui_book_button6_on{ nullptr };
    GraphicsImage *ui_book_button5_off{ nullptr };
    GraphicsImage *ui_book_button5_on{ nullptr };
    GraphicsImage *ui_book_button4_off{ nullptr };
    GraphicsImage *ui_book_button4_on{ nullptr };
    GraphicsImage *ui_book_button3_off{ nullptr };
    GraphicsImage *ui_book_button3_on{ nullptr };
    GraphicsImage *ui_book_button2_off{ nullptr };
    GraphicsImage *ui_book_button2_on{ nullptr };
    GraphicsImage *ui_book_button1_off{ nullptr };
    GraphicsImage *ui_book_button1_on{ nullptr };

    GraphicsImage *ui_book_map_frame{ nullptr };
    GraphicsImage *ui_book_quest_div_bar{ nullptr };

    bool _bookButtonClicked{ false };
    BookButtonAction _bookButtonAction;

 private:
    /**
     * @offset 0x411AAA
     */
    void initializeFonts();
};


class GUIWindow_BooksButtonOverlay : public GUIWindow {
 public:
    GUIWindow_BooksButtonOverlay(Pointi position, Sizei dimensions, GUIButton *button, std::string_view hint = {});

    virtual void Update() override;

 private:
    GUIButton *_button = nullptr;
};
