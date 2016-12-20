#pragma once
#include "GUI/GUIWindow.h"

struct GUIWindow_Spellbook : public GUIWindow
{
             GUIWindow_Spellbook();
    virtual ~GUIWindow_Spellbook() {}

    virtual void Update();
    virtual void Release();

    void OpenSpellbook();
    void OpenSpellbookPage(int page);
};


class Image;
extern Image *ui_spellbook_btn_quckspell;
extern Image *ui_spellbook_btn_quckspell_click;
extern Image *ui_spellbook_btn_close;
extern Image *ui_spellbook_btn_close_click;