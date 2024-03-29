#pragma once

#include <array>

#include "Engine/Spells/SpellEnums.h"

#include "GUI/GUIWindow.h"

class GUIWindow_Spellbook : public GUIWindow {
 public:
    GUIWindow_Spellbook();
    virtual ~GUIWindow_Spellbook() {}

    virtual void Update() override;
    virtual void Release() override;

    void openSpellbookPage(MagicSchool page);

 protected:
    void loadSpellbook();
    void openSpellbook();
    void initializeTextures();
    void drawCurrentSchoolBackground();
    void onCloseSpellBook();
    void onCloseSpellBookPage();

    GraphicsImage *ui_spellbook_btn_quckspell = nullptr;
    GraphicsImage *ui_spellbook_btn_quckspell_click = nullptr;
    GraphicsImage *ui_spellbook_btn_close = nullptr;
    GraphicsImage *ui_spellbook_btn_close_click = nullptr;

    IndexedArray<GraphicsImage *, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> ui_spellbook_school_backgrounds = {};
    IndexedArray<std::array<GraphicsImage *, 2>, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> ui_spellbook_school_tabs = {};

    std::array<GraphicsImage *, 12> SBPageCSpellsTextureList{};
    std::array<GraphicsImage *, 12> SBPageSSpellsTextureList{};
};

extern SpellId spellbookSelectedSpell;
