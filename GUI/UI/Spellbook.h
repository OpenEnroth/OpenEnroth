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


extern class Image *ui_spellbook_btn_quckspell;
extern class Image *ui_spellbook_btn_quckspell_click;
extern class Image *ui_spellbook_btn_close;
extern class Image *ui_spellbook_btn_close_click;