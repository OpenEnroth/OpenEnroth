#pragma once
#include "GUI/GUIWindow.h"

class GUIWindow_Spellbook : public GUIWindow {
 public:
    GUIWindow_Spellbook();
    virtual ~GUIWindow_Spellbook() {}

    virtual void Update();
    virtual void Release();

    void OpenSpellbook();
    void OpenSpellbookPage(int page);
};

class GraphicsImage;
extern GraphicsImage *ui_spellbook_btn_quckspell;
extern GraphicsImage *ui_spellbook_btn_quckspell_click;
extern GraphicsImage *ui_spellbook_btn_close;
extern GraphicsImage *ui_spellbook_btn_close_click;
