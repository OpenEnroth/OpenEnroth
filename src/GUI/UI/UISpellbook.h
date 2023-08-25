#pragma once

#include <stdint.h>
#include <array>
#include "Engine/Spells/SpellEnums.h"
#include "GUI/GUIWindow.h"

class GraphicsImage;
enum SPELL_TYPE : uint8_t;

class GUIWindow_Spellbook : public GUIWindow {
 public:
    GUIWindow_Spellbook();
    virtual ~GUIWindow_Spellbook() {}

    virtual void Update() override;
    virtual void Release() override;

    // TODO(captainurist): turn parameter into an enum
    void openSpellbookPage(int page);

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

    std::array<GraphicsImage *, 9> ui_spellbook_school_backgrounds{};
    std::array<std::array<GraphicsImage *, 2>, 9> ui_spellbook_school_tabs{};

    std::array<GraphicsImage *, 12> SBPageCSpellsTextureList{};
    std::array<GraphicsImage *, 12> SBPageSSpellsTextureList{};
};

extern SPELL_TYPE spellbookSelectedSpell;
