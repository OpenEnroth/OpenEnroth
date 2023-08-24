#include "GUI/UI/UISpellbook.h"

#include <string>

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Time.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Random/Random.h"

std::array<std::string, 9> spellbook_texture_filename_suffices = {{"f", "a", "w", "e", "s", "m", "b", "l", "d"}};

std::array<std::array<unsigned char, 12>, 9> pSpellbookSpellIndices = {{
      {0, 3, 1, 8, 11, 7, 4, 10, 6, 2, 5, 9},
      {0, 11, 2, 9, 6, 8, 5, 10, 3, 7, 1, 4},
      {0, 4, 8, 9, 1, 10, 3, 11, 7, 6, 2, 5},
      {0, 7, 10, 8, 2, 11, 1, 5, 3, 6, 4, 9},
      {0, 5, 10, 11, 7, 2, 8, 1, 4, 9, 3, 6},
      {0, 5, 9, 8, 3, 7, 6, 4, 1, 11, 2, 10},
      {0, 1, 6, 9, 3, 5, 8, 11, 7, 10, 4, 2},
      {0, 1, 10, 11, 9, 4, 3, 6, 5, 7, 8, 2},
      {0, 9, 3, 7, 1, 5, 2, 10, 11, 8, 6, 4}
}};

std::array<std::string, 9> texNames = {{"SBFB00", "SBAB00", "SBWB00", "SBEB00", "SBSB00", "SBMB00", "SBBB00", "SBLB00", "SBDB00"}};

std::array<std::array<int, 2>, 9> texture_tab_coord1 = {{
        {406, 9},   {406, 46},  {406, 84},  {406, 121}, {407, 158},
        {405, 196}, {405, 234}, {405, 272}, {405, 309}
}};

std::array<std::array<int, 2>, 9> texture_tab_coord0 = {{
        {415, 10},  {415, 46},  {415, 83},  {415, 121}, {415, 158},
        {416, 196}, {416, 234}, {416, 271}, {416, 307}
}};

SPELL_TYPE spellbookSelectedSpell;

GUIWindow_Spellbook::GUIWindow_Spellbook() : GUIWindow(WINDOW_SpellBook, {0, 0}, render->GetRenderDimensions(), 0) {
    current_screen_type = SCREEN_SPELL_BOOK;
    pEventTimer->Pause();

    initializeTextures();
    openSpellbook();

    // Sound 48 is absent in MM7
    pAudioPlayer->playUISound(SOUND_48);
}

void GUIWindow_Spellbook::openSpellbookPage(int page) {
    onCloseSpellBookPage();
    pParty->activeCharacter().lastOpenedSpellbookPage = page;
    openSpellbook();
    pAudioPlayer->playUISound(vrng->randomBool() ? SOUND_TurnPage2 : SOUND_TurnPage1);
}

void GUIWindow_Spellbook::openSpellbook() {
    int pageSpells = 0;
    const Character &player = pParty->activeCharacter();

    loadSpellbook();

    const CharacterSpellbookChapter *chapter = &player.spellbook.pChapters[player.lastOpenedSpellbookPage];
    for (int i = 0; i < 11; ++i) {
        if (!chapter->bIsSpellAvailable[i] && !engine->config->debug.AllMagic.value())
            continue;

        int page = player.lastOpenedSpellbookPage;
        CreateButton(fmt::format("SpellBook_Spell{}", i),
                     {pViewport->uViewportTL_X + pIconPos[page][pSpellbookSpellIndices[page][i + 1]].Xpos,
                     pViewport->uViewportTL_Y + pIconPos[page][pSpellbookSpellIndices[page][i + 1]].Ypos},
                     SBPageSSpellsTextureList[i + 1]->size(), 1, UIMSG_Spellbook_ShowHightlightedSpellInfo,
                     UIMSG_SelectSpell, i);
        pageSpells++;
    }

    CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_SpellBook_PressTab, 0, Io::InputAction::CharCycle);
    if (pageSpells) {
        setKeyboardControlGroup(pageSpells, true, 0, 0);
    }

    if (player.pActiveSkills[CHARACTER_SKILL_FIRE] || engine->config->debug.AllMagic.value())
        CreateButton({399, 10}, {50, 36}, 1, 0, UIMSG_OpenSpellbookPage, 0, Io::InputAction::Invalid, localization->GetSpellSchoolName(0));
    if (player.pActiveSkills[CHARACTER_SKILL_AIR] || engine->config->debug.AllMagic.value())
        CreateButton({399, 46}, {50, 36}, 1, 0, UIMSG_OpenSpellbookPage, 1, Io::InputAction::Invalid, localization->GetSpellSchoolName(1));
    if (player.pActiveSkills[CHARACTER_SKILL_WATER] || engine->config->debug.AllMagic.value())
        CreateButton({399, 83}, {50, 36}, 1, 0, UIMSG_OpenSpellbookPage, 2, Io::InputAction::Invalid, localization->GetSpellSchoolName(2));
    if (player.pActiveSkills[CHARACTER_SKILL_EARTH] || engine->config->debug.AllMagic.value())
        CreateButton({399, 121}, {50, 36}, 1, 0, UIMSG_OpenSpellbookPage, 3, Io::InputAction::Invalid, localization->GetSpellSchoolName(3));
    if (player.pActiveSkills[CHARACTER_SKILL_SPIRIT] || engine->config->debug.AllMagic.value())
        CreateButton({399, 158}, {50, 36}, 1, 0, UIMSG_OpenSpellbookPage, 4, Io::InputAction::Invalid, localization->GetSpellSchoolName(5));
    if (player.pActiveSkills[CHARACTER_SKILL_MIND] || engine->config->debug.AllMagic.value())
        CreateButton({400, 196}, {50, 36}, 1, 0, UIMSG_OpenSpellbookPage, 5, Io::InputAction::Invalid, localization->GetSpellSchoolName(4));
    if (player.pActiveSkills[CHARACTER_SKILL_BODY] || engine->config->debug.AllMagic.value())
        CreateButton({400, 234}, {50, 36}, 1, 0, UIMSG_OpenSpellbookPage, 6, Io::InputAction::Invalid, localization->GetSpellSchoolName(6));
    if (player.pActiveSkills[CHARACTER_SKILL_LIGHT] || engine->config->debug.AllMagic.value())
        CreateButton({400, 271}, {50, 36}, 1, 0, UIMSG_OpenSpellbookPage, 7, Io::InputAction::Invalid, localization->GetSpellSchoolName(7));
    if (player.pActiveSkills[CHARACTER_SKILL_DARK] || engine->config->debug.AllMagic.value())
        CreateButton({400, 307}, {50, 36}, 1, 0, UIMSG_OpenSpellbookPage, 8, Io::InputAction::Invalid, localization->GetSpellSchoolName(8));

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

    int page = 0;
    for (CharacterSkillType i : allMagicSkills()) {
        if (player.pActiveSkills[i] || engine->config->debug.AllMagic.value()) {
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

            const CharacterSpellbookChapter *chapter = &player.spellbook.pChapters[player.lastOpenedSpellbookPage];
            for (int i = 0; i < 11; ++i) {
                if (chapter->bIsSpellAvailable[i] || engine->config->debug.AllMagic.value()) {
                    // this should check if player knows spell
                    if (SBPageSSpellsTextureList[i + 1]) {
                        SPELL_TYPE spellFlat = static_cast<SPELL_TYPE>(11 * player.lastOpenedSpellbookPage + i + 1);
                        GraphicsImage *pTexture = (spellbookSelectedSpell == spellFlat) ? SBPageCSpellsTextureList[i + 1] : SBPageSSpellsTextureList[i + 1];
                        if (pTexture) {
                            SpellBookIconPos &iconPos = pIconPos[player.lastOpenedSpellbookPage][pSpellbookSpellIndices[player.lastOpenedSpellbookPage][i + 1]];

                            pX_coord = pViewport->uViewportTL_X + iconPos.Xpos;
                            pY_coord = pViewport->uViewportTL_Y + iconPos.Ypos;

                            render->DrawTextureNew(pX_coord / 640.0f, pY_coord / 480.0f, pTexture);
                            render->ZDrawTextureAlpha(iconPos.Xpos / 640.0f, iconPos.Ypos / 480.0f, pTexture, i + 1);
                        }
                    }
                }
            }
        }

        page++;
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
    // TODO(captainurist): encapsulate this enum arithmetic properly
    // TODO(Nik-RE-dev): turn variable into an enum
    int spellSchool = pParty->activeCharacter().lastOpenedSpellbookPage;
    if (pParty->activeCharacter().uQuickSpell != SPELL_NONE && pParty->activeCharacter().uQuickSpell / 11 == spellSchool)
        spellbookSelectedSpell = pParty->activeCharacter().uQuickSpell;
    else
        spellbookSelectedSpell = SPELL_NONE;

    for (int i = 1; i <= 11; ++i) {
        if (pParty->activeCharacter().spellbook.pChapters[spellSchool].bIsSpellAvailable[i - 1] || engine->config->debug.AllMagic.value()) {
            std::string pContainer;

            pContainer = fmt::format("SB{}S{:02}", spellbook_texture_filename_suffices[spellSchool], pSpellbookSpellIndices[spellSchool][i]);
            SBPageSSpellsTextureList[i] = assets->getImage_Solid(pContainer);

            pContainer = fmt::format("SB{}C{:02}", spellbook_texture_filename_suffices[spellSchool], pSpellbookSpellIndices[spellSchool][i]);
            SBPageCSpellsTextureList[i] = assets->getImage_Solid(pContainer);
        }
    }
}

void GUIWindow_Spellbook::drawCurrentSchoolBackground() {
    int pTexID = 0;
    if (pParty->hasActiveCharacter()) {
        pTexID = pParty->activeCharacter().lastOpenedSpellbookPage;
    }
    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, ui_spellbook_school_backgrounds[pTexID]);

    render->DrawTextureNew(476 / 640.0f, 450 / 480.0f, ui_spellbook_btn_quckspell);
    render->DrawTextureNew(561 / 640.0f, 450 / 480.0f, ui_spellbook_btn_close);
}

void GUIWindow_Spellbook::initializeTextures() {
    pAudioPlayer->playUISound(SOUND_openbook);

    ui_spellbook_btn_close = assets->getImage_Solid("ib-m5-u");
    ui_spellbook_btn_close_click = assets->getImage_Solid("ib-m5-d");
    ui_spellbook_btn_quckspell = assets->getImage_Solid("ib-m6-u");
    ui_spellbook_btn_quckspell_click = assets->getImage_Solid("ib-m6-d");

    for (int i = 0; i < 9; ++i) {
        ui_spellbook_school_backgrounds[i] = assets->getImage_ColorKey(texNames[i]);
        ui_spellbook_school_tabs[i][0] = assets->getImage_Alpha(fmt::format("tab{}a", i + 1));
        ui_spellbook_school_tabs[i][1] = assets->getImage_Alpha(fmt::format("tab{}b", i + 1));
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

    for (int i = 0; i < 9; ++i) {
        if (ui_spellbook_school_backgrounds[i]) {
            ui_spellbook_school_backgrounds[i]->Release();
            ui_spellbook_school_backgrounds[i] = nullptr;
        }

        if (ui_spellbook_school_tabs[i][0]) {
            ui_spellbook_school_tabs[i][0]->Release();
            ui_spellbook_school_tabs[i][0] = nullptr;
        }
        if (ui_spellbook_school_tabs[i][1]) {
            ui_spellbook_school_tabs[i][1]->Release();
            ui_spellbook_school_tabs[i][1] = nullptr;
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
