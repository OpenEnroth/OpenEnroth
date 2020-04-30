#pragma once
#include "GUI/GUIWindow.h"

class GUIWindow_Book : public GUIWindow {
 public:
    GUIWindow_Book();
    virtual ~GUIWindow_Book() {}

    virtual void Release();

 protected:
    void BasicBookInitialization();

 private:
    void InitializeFonts();
};


class GUIWindow_BooksButtonOverlay : public GUIWindow {
 public:
    GUIWindow_BooksButtonOverlay(unsigned int x, unsigned int y, unsigned int width, unsigned int height, GUIButton *button, const String &hint = String()) :
        GUIWindow(WINDOW_BooksButtonOverlay, x, y, width, height, button, hint)
    {}
    virtual ~GUIWindow_BooksButtonOverlay() {}

    virtual void Update();
};

class Image;
extern Image *ui_book_button8_off;
extern Image *ui_book_button8_on;
extern Image *ui_book_button7_off;
extern Image *ui_book_button7_on;
extern Image *ui_book_button6_off;
extern Image *ui_book_button6_on;
extern Image *ui_book_button5_off;
extern Image *ui_book_button5_on;
extern Image *ui_book_button4_off;
extern Image *ui_book_button4_on;
extern Image *ui_book_button3_off;
extern Image *ui_book_button3_on;
extern Image *ui_book_button2_off;
extern Image *ui_book_button2_on;
extern Image *ui_book_button1_off;
extern Image *ui_book_button1_on;

extern Image *ui_book_map_frame;
extern Image *ui_book_quest_div_bar;
