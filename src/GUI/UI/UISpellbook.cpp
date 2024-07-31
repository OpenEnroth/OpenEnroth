#include "GUI/UI/UISpellbook.h"

#include <string>

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Random/Random.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Spells/SpellEnumFunctions.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Time/Timer.h"

#include "GUI/GUIButton.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

static constexpr IndexedArray<const char *, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> spellbook_texture_filename_suffices = {
    {MAGIC_SCHOOL_FIRE,   "f"},
    {MAGIC_SCHOOL_AIR,    "a"},
    {MAGIC_SCHOOL_WATER,  "w"},
    {MAGIC_SCHOOL_EARTH,  "e"},
    {MAGIC_SCHOOL_SPIRIT, "s"},
    {MAGIC_SCHOOL_MIND,   "m"},
    {MAGIC_SCHOOL_BODY,   "b"},
    {MAGIC_SCHOOL_LIGHT,  "l"},
    {MAGIC_SCHOOL_DARK,   "d"}
};

static constexpr IndexedArray<std::array<unsigned char, 12>, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> pSpellbookSpellIndices = {{
    {MAGIC_SCHOOL_FIRE,     {0, 3, 1, 8, 11, 7, 4, 10, 6, 2, 5, 9}},
    {MAGIC_SCHOOL_AIR,      {0, 11, 2, 9, 6, 8, 5, 10, 3, 7, 1, 4}},
    {MAGIC_SCHOOL_WATER,    {0, 4, 8, 9, 1, 10, 3, 11, 7, 6, 2, 5}},
    {MAGIC_SCHOOL_EARTH,    {0, 7, 10, 8, 2, 11, 1, 5, 3, 6, 4, 9}},
    {MAGIC_SCHOOL_SPIRIT,   {0, 5, 10, 11, 7, 2, 8, 1, 4, 9, 3, 6}},
    {MAGIC_SCHOOL_MIND,     {0, 5, 9, 8, 3, 7, 6, 4, 1, 11, 2, 10}},
    {MAGIC_SCHOOL_BODY,     {0, 1, 6, 9, 3, 5, 8, 11, 7, 10, 4, 2}},
    {MAGIC_SCHOOL_LIGHT,    {0, 1, 10, 11, 9, 4, 3, 6, 5, 7, 8, 2}},
    {MAGIC_SCHOOL_DARK,     {0, 9, 3, 7, 1, 5, 2, 10, 11, 8, 6, 4}}
}};

static constexpr IndexedArray<const char *, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> texNames = {
    {MAGIC_SCHOOL_FIRE,   "SBFB00"},
    {MAGIC_SCHOOL_AIR,    "SBAB00"},
    {MAGIC_SCHOOL_WATER,  "SBWB00"},
    {MAGIC_SCHOOL_EARTH,  "SBEB00"},
    {MAGIC_SCHOOL_SPIRIT, "SBSB00"},
    {MAGIC_SCHOOL_MIND,   "SBMB00"},
    {MAGIC_SCHOOL_BODY,   "SBBB00"},
    {MAGIC_SCHOOL_LIGHT,  "SBLB00"},
    {MAGIC_SCHOOL_DARK,   "SBDB00"}
};

static constexpr IndexedArray<std::array<int, 2>, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> texture_tab_coord1 = {{
    {MAGIC_SCHOOL_FIRE,     {406, 9}},
    {MAGIC_SCHOOL_AIR,      {406, 46}},
    {MAGIC_SCHOOL_WATER,    {406, 84}},
    {MAGIC_SCHOOL_EARTH,    {406, 121}},
    {MAGIC_SCHOOL_SPIRIT,   {407, 158}},
    {MAGIC_SCHOOL_MIND,     {405, 196}},
    {MAGIC_SCHOOL_BODY,     {405, 234}},
    {MAGIC_SCHOOL_LIGHT,    {405, 272}},
    {MAGIC_SCHOOL_DARK,     {405, 309}}
}};

static constexpr IndexedArray<std::array<int, 2>, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> texture_tab_coord0 = {{
    {MAGIC_SCHOOL_FIRE,     {415, 10}},
    {MAGIC_SCHOOL_AIR,      {415, 46}},
    {MAGIC_SCHOOL_WATER,    {415, 83}},
    {MAGIC_SCHOOL_EARTH,    {415, 121}},
    {MAGIC_SCHOOL_SPIRIT,   {415, 158}},
    {MAGIC_SCHOOL_MIND,     {416, 196}},
    {MAGIC_SCHOOL_BODY,     {416, 234}},
    {MAGIC_SCHOOL_LIGHT,    {416, 271}},
    {MAGIC_SCHOOL_DARK,     {416, 307}}
}};

SpellId spellbookSelectedSpell;

GUIWindow_Spellbook::GUIWindow_Spellbook() : GUIWindow(WINDOW_SpellBook, {0, 0}, render->GetRenderDimensions()) {
    current_screen_type = SCREEN_SPELL_BOOK;
    pEventTimer->setPaused(true);

    initializeTextures();
    openSpellbook();

    // Sound 48 is absent in MM7
    pAudioPlayer->playUISound(SOUND_48);
}

void GUIWindow_Spellbook::openSpellbookPage(MagicSchool page) {
    onCloseSpellBookPage();
    pParty->activeCharacter().lastOpenedSpellbookPage = page;
    openSpellbook();
    pAudioPlayer->playUISound(vrng->randomBool() ? SOUND_TurnPage2 : SOUND_TurnPage1);
}

void GUIWindow_Spellbook::openSpellbook() {
    int pageSpells = 0;
    const Character &player = pParty->activeCharacter();

    loadSpellbook();

    MagicSchool chapter = player.lastOpenedSpellbookPage;
    for (SpellId spell : spellsForMagicSchool(chapter)) {
        if (!player.bHaveSpell[spell] && !engine->config->debug.AllMagic.value())
            continue;

        int index = spellIndexInMagicSchool(spell);
        CreateButton(fmt::format("SpellBook_Spell{}", index),
                     {pViewport->uViewportTL_X + pIconPos[chapter][pSpellbookSpellIndices[chapter][index + 1]].Xpos,
                     pViewport->uViewportTL_Y + pIconPos[chapter][pSpellbookSpellIndices[chapter][index + 1]].Ypos},
                     SBPageSSpellsTextureList[index + 1]->size(), 1, UIMSG_Spellbook_ShowHightlightedSpellInfo,
                     UIMSG_SelectSpell, std::to_underlying(spell));
        pageSpells++;
    }

    CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_SpellBook_PressTab, 0, Io::InputAction::CharCycle);
    if (pageSpells) {
        setKeyboardControlGroup(pageSpells, true, 0, 0);
    }

    static constexpr IndexedArray<Pointi, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> buttonPositions = {
        {MAGIC_SCHOOL_FIRE,     {399, 10}},
        {MAGIC_SCHOOL_AIR,      {399, 46}},
        {MAGIC_SCHOOL_WATER,    {399, 83}},
        {MAGIC_SCHOOL_EARTH,    {399, 121}},
        {MAGIC_SCHOOL_SPIRIT,   {399, 158}},
        {MAGIC_SCHOOL_MIND,     {400, 196}},
        {MAGIC_SCHOOL_BODY,     {400, 234}},
        {MAGIC_SCHOOL_LIGHT,    {400, 271}},
        {MAGIC_SCHOOL_DARK,     {400, 307}},
    };

    for (MagicSchool school : allMagicSchools())
        if (player.pActiveSkills[skillForMagicSchool(school)] || engine->config->debug.AllMagic.value())
            CreateButton(buttonPositions[school], {50, 36}, 1, 0, UIMSG_OpenSpellbookPage, std::to_underlying(school),
                         Io::InputAction::Invalid, localization->GetSpellSchoolName(school));

    pBtn_InstallRemoveSpell = CreateButton({476, 450}, ui_spellbook_btn_quckspell->size(), 1, UIMSG_HintSelectRemoveQuickSpellBtn,
                                           UIMSG_ClickInstallRemoveQuickSpellBtn, 0, Io::InputAction::Invalid, "", {ui_spellbook_btn_quckspell_click});
    pBtn_CloseBook = CreateButton({561, 450}, ui_spellbook_btn_close->size(), 1, 0, UIMSG_Escape, 0, Io::InputAction::Invalid,
                                  localization->GetString(LSTR_DIALOGUE_EXIT), {ui_spellbook_btn_close_click});
}

void GUIWindow_Spellbook::Update() {
    const Character &player = pParty->activeCharacter();
    unsigned int pX_coord, pY_coord;

    drawCurrentSchoolBackground();

    render->ClearZBuffer();

    for (MagicSchool page : allMagicSchools()) {
        CharacterSkillType skill = skillForMagicSchool(page);

        if (player.pActiveSkills[skill] || engine->config->debug.AllMagic.value()) {
            auto pPageTexture = ui_spellbook_school_tabs[page][0];
            if (player.lastOpenedSpellbookPage == page) {
                pPageTexture = ui_spellbook_school_tabs[page][1];
                pX_coord = texture_tab_coord1[page][0];
                pY_coord = texture_tab_coord1[page][1];
            } else {
                pPageTexture = ui_spellbook_school_tabs[page][0];
                pX_coord = texture_tab_coord0[page][0];
                pY_coord = texture_tab_coord0[page][1];
            }
            render->DrawTextureNew(pX_coord / 640.0f, pY_coord / 480.0f, pPageTexture);

            for (SpellId spell : spellsForMagicSchool(player.lastOpenedSpellbookPage)) {
                int index = spellIndexInMagicSchool(spell);
                if (player.bHaveSpell[spell] || engine->config->debug.AllMagic.value()) {
                    // this should check if player knows spell
                    if (SBPageSSpellsTextureList[index + 1]) {
                        GraphicsImage *pTexture = (spellbookSelectedSpell == spell) ? SBPageCSpellsTextureList[index + 1] : SBPageSSpellsTextureList[index + 1];
                        if (pTexture) {
                            SpellBookIconPos &iconPos = pIconPos[player.lastOpenedSpellbookPage][pSpellbookSpellIndices[player.lastOpenedSpellbookPage][index + 1]];

                            pX_coord = pViewport->uViewportTL_X + iconPos.Xpos;
                            pY_coord = pViewport->uViewportTL_Y + iconPos.Ypos;

                            render->DrawTextureNew(pX_coord / 640.0f, pY_coord / 480.0f, pTexture);
                            render->ZDrawTextureAlpha(iconPos.Xpos / 640.0f, iconPos.Ypos / 480.0f, pTexture, index + 1);
                        }
                    }
                }
            }
        }
    }

    Pointi pt = mouse->GetCursorPos();
    Sizei renDims = render->GetRenderDimensions();
    if (pt.x < renDims.w && pt.y < renDims.h) {
        int idx = render->pActiveZBuffer[pt.x + pt.y * render->GetRenderDimensions().w] & 0xFFFF;
        if (idx) {
            if (SBPageCSpellsTextureList[idx]) {
                SpellBookIconPos &iconPos = pIconPos[player.lastOpenedSpellbookPage][pSpellbookSpellIndices[player.lastOpenedSpellbookPage][idx]];

                pX_coord = pViewport->uViewportTL_X + iconPos.Xpos;
                pY_coord = pViewport->uViewportTL_Y + iconPos.Ypos;

                render->DrawTextureNew(pX_coord / 640.0f, pY_coord / 480.0f, SBPageCSpellsTextureList[idx]);
            }
        }
    }
}

void GUIWindow_Spellbook::Release() {
    onCloseSpellBookPage();
    onCloseSpellBook();

    GUIWindow::Release();
}

void GUIWindow_Spellbook::loadSpellbook() {
    MagicSchool page = pParty->activeCharacter().lastOpenedSpellbookPage;
    if (pParty->activeCharacter().uQuickSpell != SPELL_NONE && magicSchoolForSpell(pParty->activeCharacter().uQuickSpell) == page)
        spellbookSelectedSpell = pParty->activeCharacter().uQuickSpell;
    else
        spellbookSelectedSpell = SPELL_NONE;

    for (SpellId spell : spellsForMagicSchool(page)) {
        if (pParty->activeCharacter().bHaveSpell[spell] || engine->config->debug.AllMagic.value()) {
            int index = spellIndexInMagicSchool(spell);
            std::string pContainer;

            pContainer = fmt::format("SB{}S{:02}", spellbook_texture_filename_suffices[page], pSpellbookSpellIndices[page][index + 1]);
            SBPageSSpellsTextureList[index + 1] = assets->getImage_Solid(pContainer);

            pContainer = fmt::format("SB{}C{:02}", spellbook_texture_filename_suffices[page], pSpellbookSpellIndices[page][index + 1]);
            SBPageCSpellsTextureList[index + 1] = assets->getImage_Solid(pContainer);
        }
    }
}

void GUIWindow_Spellbook::drawCurrentSchoolBackground() {
    MagicSchool page = MAGIC_SCHOOL_FIRE;
    if (pParty->hasActiveCharacter()) {
        page = pParty->activeCharacter().lastOpenedSpellbookPage;
    }
    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, ui_spellbook_school_backgrounds[page]);

    render->DrawTextureNew(476 / 640.0f, 450 / 480.0f, ui_spellbook_btn_quckspell);
    render->DrawTextureNew(561 / 640.0f, 450 / 480.0f, ui_spellbook_btn_close);
}

void GUIWindow_Spellbook::initializeTextures() {
    pAudioPlayer->playUISound(SOUND_openbook);

    ui_spellbook_btn_close = assets->getImage_Solid("ib-m5-u");
    ui_spellbook_btn_close_click = assets->getImage_Solid("ib-m5-d");
    ui_spellbook_btn_quckspell = assets->getImage_Solid("ib-m6-u");
    ui_spellbook_btn_quckspell_click = assets->getImage_Solid("ib-m6-d");

    for (MagicSchool page : allMagicSchools()) {
        ui_spellbook_school_backgrounds[page] = assets->getImage_ColorKey(texNames[page]);
        ui_spellbook_school_tabs[page][0] = assets->getImage_Alpha(fmt::format("tab{}a", std::to_underlying(page) + 1));
        ui_spellbook_school_tabs[page][1] = assets->getImage_Alpha(fmt::format("tab{}b", std::to_underlying(page) + 1));
    }
}

void GUIWindow_Spellbook::onCloseSpellBook() {
    if (ui_spellbook_btn_close) {
        ui_spellbook_btn_close->Release();
        ui_spellbook_btn_close = nullptr;
    }
    if (ui_spellbook_btn_close_click) {
        ui_spellbook_btn_close_click->Release();
        ui_spellbook_btn_close_click = nullptr;
    }

    if (ui_spellbook_btn_quckspell) {
        ui_spellbook_btn_quckspell->Release();
        ui_spellbook_btn_quckspell = nullptr;
    }
    if (ui_spellbook_btn_quckspell_click) {
        ui_spellbook_btn_quckspell_click->Release();
        ui_spellbook_btn_quckspell_click = nullptr;
    }

    for (MagicSchool page : allMagicSchools()) {
        if (ui_spellbook_school_backgrounds[page]) {
            ui_spellbook_school_backgrounds[page]->Release();
            ui_spellbook_school_backgrounds[page] = nullptr;
        }

        if (ui_spellbook_school_tabs[page][0]) {
            ui_spellbook_school_tabs[page][0]->Release();
            ui_spellbook_school_tabs[page][0] = nullptr;
        }
        if (ui_spellbook_school_tabs[page][1]) {
            ui_spellbook_school_tabs[page][1]->Release();
            ui_spellbook_school_tabs[page][1] = nullptr;
        }
    }

    pAudioPlayer->playUISound(SOUND_closebook);
}

void GUIWindow_Spellbook::onCloseSpellBookPage() {
    for (unsigned int i = 1; i <= 11; i++) {
        if (SBPageCSpellsTextureList[i]) {
            SBPageCSpellsTextureList[i]->Release();
            SBPageCSpellsTextureList[i] = nullptr;
        }
        if (SBPageSSpellsTextureList[i]) {
            SBPageSSpellsTextureList[i]->Release();
            SBPageSSpellsTextureList[i] = nullptr;
        }
    }
    pGUIWindow_CurrentMenu->DeleteButtons();
}
