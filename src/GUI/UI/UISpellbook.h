#pragma once

#include <array>
#include <memory>

#include "Engine/Spells/SpellEnums.h"

#include "GUI/GUIWindow.h"

class GUIWindow_Spellbook : public GUIWindow {
 public:
    GUIWindow_Spellbook();
    virtual ~GUIWindow_Spellbook();

    virtual void Update() override;

    void openSpellbookPage(MagicSchool page);

 protected:
    void loadSpellbook();
    void openSpellbook();
    void initializeTextures();
    void drawCurrentSchoolBackground();
    void onCloseSpellBook();
    void onCloseSpellBookPage();

    std::shared_ptr<GraphicsImage> ui_spellbook_btn_quckspell;
    std::shared_ptr<GraphicsImage> ui_spellbook_btn_quckspell_click;
    std::shared_ptr<GraphicsImage> ui_spellbook_btn_close;
    std::shared_ptr<GraphicsImage> ui_spellbook_btn_close_click;
    // Real MM8 art ("stssu", icons.lod) for the 3rd left-edge icon slot between Quick
    // Spell and Close - drawn decoratively only, its function is unconfirmed (see initializeTextures()).
    std::shared_ptr<GraphicsImage> ui_spellbook_unconfirmed_icon;

    IndexedArray<std::shared_ptr<GraphicsImage>, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> ui_spellbook_school_backgrounds;
    IndexedArray<std::array<std::shared_ptr<GraphicsImage>, 2>, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> ui_spellbook_school_tabs;

    std::array<std::shared_ptr<GraphicsImage>, 12> SBPageCSpellsTextureList;
    std::array<std::shared_ptr<GraphicsImage>, 12> SBPageSSpellsTextureList;
};

extern SpellId spellbookSelectedSpell;
