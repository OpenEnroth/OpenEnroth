#include "GUI/UI/Spellbook.h"

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Time.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Random/Random.h"


void InitializeSpellBookTextures();
void OnCloseSpellBookPage();
void OnCloseSpellBook();
void LoadSpellbook(unsigned int spell_school); // TODO(captainurist): turn parameter into an enum
static void BookUI_Spellbook_DrawCurrentSchoolBackground();

std::array<const char *, 9> spellbook_texture_filename_suffices = {
    {"f", "a", "w", "e", "s", "m", "b", "l", "d"}};

std::array<std::array<unsigned char, 12>, 9>
    pSpellbookSpellIndices =  // 4E2430    from pSpellbookSpellIndices[9][12]
    {{                        // 0   1   2   3  4    5   6  7    8  9   10  11
      {0, 3, 1, 8, 11, 7, 4, 10, 6, 2, 5, 9},
      {0, 11, 2, 9, 6, 8, 5, 10, 3, 7, 1, 4},
      {0, 4, 8, 9, 1, 10, 3, 11, 7, 6, 2, 5},
      {0, 7, 10, 8, 2, 11, 1, 5, 3, 6, 4, 9},
      {0, 5, 10, 11, 7, 2, 8, 1, 4, 9, 3, 6},
      {0, 5, 9, 8, 3, 7, 6, 4, 1, 11, 2, 10},
      {0, 1, 6, 9, 3, 5, 8, 11, 7, 10, 4, 2},
      {0, 1, 10, 11, 9, 4, 3, 6, 5, 7, 8, 2},
      {0, 9, 3, 7, 1, 5, 2, 10, 11, 8, 6, 4}}};

Image *ui_spellbook_btn_quckspell = nullptr;
Image *ui_spellbook_btn_quckspell_click = nullptr;
Image *ui_spellbook_btn_close = nullptr;
Image *ui_spellbook_btn_close_click = nullptr;

std::array<Image *, 12> SBPageCSpellsTextureList;
std::array<Image *, 12> SBPageSSpellsTextureList;

std::array<Image *, 9> ui_spellbook_school_backgrounds;
std::array<std::array<Image *, 2>, 9> ui_spellbook_school_tabs;

GUIWindow_Spellbook::GUIWindow_Spellbook()
    : GUIWindow(WINDOW_SpellBook, {0, 0}, render->GetRenderDimensions(), 0) {
    current_screen_type = CURRENT_SCREEN::SCREEN_SPELL_BOOK;
    pEventTimer->Pause();

    InitializeSpellBookTextures();
    OpenSpellbook();

    // Sound 48 is absent in MM7
    pAudioPlayer->playUISound(SOUND_48);
}

void GUIWindow_Spellbook::OpenSpellbookPage(int page) {
    OnCloseSpellBookPage();
    pParty->activeCharacter().lastOpenedSpellbookPage = page;
    OpenSpellbook();
    pAudioPlayer->playUISound(vrng->randomBool() ? SOUND_TurnPage2 : SOUND_TurnPage1);
}

void GUIWindow_Spellbook::OpenSpellbook() {
    Player *pPlayer;  // edi@1
    // GUIWindow *pWindow; // esi@1
    // unsigned int v3; // ebp@1
    int v4;  // eax@3
    /// GUIButton *result; // eax@25
    int a2;  // [sp+10h] [bp-8h]@1
    // int v7; // [sp+14h] [bp-4h]@1

    pPlayer = &pParty->activeCharacter();
    // pWindow = this;
    LoadSpellbook(pPlayer->lastOpenedSpellbookPage);
    // v3 = 0;
    a2 = 0;

    PlayerSpellbookChapter *chapter = &pPlayer->spellbook.pChapters[pPlayer->lastOpenedSpellbookPage];
    for (uint i = 0; i < 11; ++i) {
        if (!chapter->bIsSpellAvailable[i] && !engine->config->debug.AllMagic.value())
            continue;

        v4 = pPlayer->lastOpenedSpellbookPage;
        // v4 = (12 * pPlayer->lastOpenedSpellbookPage +
        // pSpellbookSpellIndices[pPlayer->lastOpenedSpellbookPage][i + 1]);
        CreateButton(fmt::format("SpellBook_Spell{}", i),
                     {pViewport->uViewportTL_X + pIconPos[v4][pSpellbookSpellIndices[v4][i + 1]].Xpos,
                     pViewport->uViewportTL_Y + pIconPos[v4][pSpellbookSpellIndices[v4][i + 1]].Ypos},  // dword_4E20D0
                     {SBPageSSpellsTextureList[i + 1]->GetWidth(), SBPageSSpellsTextureList[i + 1]->GetHeight()}, 1, 79,
                     UIMSG_SelectSpell, i);
        ++a2;
        // ++v3;
    }
    // while ( (signed int)v3 < 11 );

    CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_SpellBook_PressTab, 0, InputAction::CharCycle);
    if (a2) _41D08F_set_keyboard_control_group(a2, 0, 0, 0);

    if (pPlayer->pActiveSkills[PLAYER_SKILL_FIRE] || engine->config->debug.AllMagic.value())
        CreateButton({399, 10}, {50, 36}, 1, 0, UIMSG_OpenSpellbookPage, 0, InputAction::Invalid, localization->GetSpellSchoolName(0));
    if (pPlayer->pActiveSkills[PLAYER_SKILL_AIR] || engine->config->debug.AllMagic.value())
        CreateButton({399, 46}, {50, 36}, 1, 0, UIMSG_OpenSpellbookPage, 1, InputAction::Invalid, localization->GetSpellSchoolName(1));
    if (pPlayer->pActiveSkills[PLAYER_SKILL_WATER] || engine->config->debug.AllMagic.value())
        CreateButton({399, 83}, {50, 36}, 1, 0, UIMSG_OpenSpellbookPage, 2, InputAction::Invalid, localization->GetSpellSchoolName(2));
    if (pPlayer->pActiveSkills[PLAYER_SKILL_EARTH] || engine->config->debug.AllMagic.value())
        CreateButton({399, 121}, {50, 36}, 1, 0, UIMSG_OpenSpellbookPage, 3, InputAction::Invalid, localization->GetSpellSchoolName(3));
    if (pPlayer->pActiveSkills[PLAYER_SKILL_SPIRIT] || engine->config->debug.AllMagic.value())
        CreateButton({399, 158}, {50, 36}, 1, 0, UIMSG_OpenSpellbookPage, 4, InputAction::Invalid, localization->GetSpellSchoolName(5));
    if (pPlayer->pActiveSkills[PLAYER_SKILL_MIND] || engine->config->debug.AllMagic.value())
        CreateButton({400, 196}, {50, 36}, 1, 0, UIMSG_OpenSpellbookPage, 5, InputAction::Invalid, localization->GetSpellSchoolName(4));
    if (pPlayer->pActiveSkills[PLAYER_SKILL_BODY] || engine->config->debug.AllMagic.value())
        CreateButton({400, 234}, {50, 36}, 1, 0, UIMSG_OpenSpellbookPage, 6, InputAction::Invalid, localization->GetSpellSchoolName(6));
    if (pPlayer->pActiveSkills[PLAYER_SKILL_LIGHT] || engine->config->debug.AllMagic.value())
        CreateButton({400, 271}, {50, 36}, 1, 0, UIMSG_OpenSpellbookPage, 7, InputAction::Invalid, localization->GetSpellSchoolName(7));
    if (pPlayer->pActiveSkills[PLAYER_SKILL_DARK] || engine->config->debug.AllMagic.value())
        CreateButton({400, 307}, {50, 36}, 1, 0, UIMSG_OpenSpellbookPage, 8, InputAction::Invalid, localization->GetSpellSchoolName(8));

    CreateButton({476, 450}, {ui_spellbook_btn_quckspell->GetWidth(), ui_spellbook_btn_quckspell->GetHeight()}, 1, 78,
        UIMSG_ClickInstallRemoveQuickSpellBtn, 0);
    pBtn_InstallRemoveSpell = CreateButton({476, 450}, {48, 32}, 1, 78,
        UIMSG_ClickInstallRemoveQuickSpellBtn, 0, InputAction::Invalid, "", {ui_spellbook_btn_quckspell_click});
    CreateButton({561, 450}, {ui_spellbook_btn_close->GetWidth(), ui_spellbook_btn_close->GetHeight()}, 1, 0,
        UIMSG_Escape, 0, InputAction::Invalid, localization->GetString(LSTR_DIALOGUE_EXIT));
    pBtn_CloseBook = CreateButton({561, 450}, {48, 32}, 1, 0,
        UIMSG_Escape, 0, InputAction::Invalid, localization->GetString(LSTR_DIALOGUE_EXIT), {ui_spellbook_btn_close_click});
}

void GUIWindow_Spellbook::Update() {
    auto player = &pParty->activeCharacter();

    Image *pTexture;        // edx@5
    int v10;                // eax@13
    unsigned int pX_coord;  // esi@18
    unsigned int pY_coord;  // edi@18

    static unsigned int texture_tab_coord1[9][2] = {
        {406, 9},   {406, 46},  {406, 84},  {406, 121}, {407, 158},
        {405, 196}, {405, 234}, {405, 272}, {405, 309}};

    static unsigned int texture_tab_coord0[9][2] = {
        {415, 10},  {415, 46},  {415, 83},  {415, 121}, {415, 158},
        {416, 196}, {416, 234}, {416, 271}, {416, 307}};

    BookUI_Spellbook_DrawCurrentSchoolBackground();

    render->ClearZBuffer();

    int page = 0;
    for (PLAYER_SKILL_TYPE i : MagicSkills()) {
        if (player->pActiveSkills[i] || engine->config->debug.AllMagic.value()) {
            auto pPageTexture = ui_spellbook_school_tabs[page][0];
            if (player->lastOpenedSpellbookPage == page) {
                pPageTexture = ui_spellbook_school_tabs[page][1];
                pX_coord = texture_tab_coord1[page][0];
                pY_coord = texture_tab_coord1[page][1];
            } else {
                pPageTexture = ui_spellbook_school_tabs[page][0];
                pX_coord = texture_tab_coord0[page][0];
                pY_coord = texture_tab_coord0[page][1];
            }
            render->DrawTextureNew(pX_coord / 640.0f, pY_coord / 480.0f,
                                        pPageTexture);

            PlayerSpellbookChapter *chapter =
                &player->spellbook.pChapters[player->lastOpenedSpellbookPage];
            for (unsigned int i = 0; i < 11; ++i) {
                if (chapter->bIsSpellAvailable[i] || engine->config->debug.AllMagic.value()) {
                        // this should check if oplayer knows spell
                    if (SBPageSSpellsTextureList[i+1]) {
                        if (quick_spell_at_page == i+1)
                            pTexture = SBPageCSpellsTextureList[i+1];
                        else
                            pTexture = SBPageSSpellsTextureList[i+1];
                        if (pTexture) {
                            pX_coord =
                                pViewport->uViewportTL_X +
                                pIconPos[player->lastOpenedSpellbookPage]
                                        [pSpellbookSpellIndices
                                             [player->lastOpenedSpellbookPage]
                                             [i+1]]
                                            .Xpos;
                            pY_coord =
                                pViewport->uViewportTL_Y +
                                pIconPos[player->lastOpenedSpellbookPage]
                                        [pSpellbookSpellIndices
                                             [player->lastOpenedSpellbookPage]
                                             [i+1]]
                                            .Ypos;

                            render->DrawTextureNew(
                                pX_coord / 640.0f, pY_coord / 480.0f, pTexture);

                            //

                            render->ZDrawTextureAlpha(
                                pIconPos[player->lastOpenedSpellbookPage]
                                        [pSpellbookSpellIndices
                                             [player->lastOpenedSpellbookPage]
                                             [i+1]]
                                            .Xpos /
                                    640.0f,
                                pIconPos[player->lastOpenedSpellbookPage]
                                        [pSpellbookSpellIndices
                                             [player->lastOpenedSpellbookPage]
                                             [i+1]]
                                            .Ypos /
                                    480.0f,
                                pTexture, i+1);
                        }
                    }
                }
            }
        }

        page++;
    }

    // if ((11 * player->lastOpenedSpellbookPage) || ((11 *
    // player->lastOpenedSpellbookPage) + 11))//??? maybe structure need fix
    //{

    // }

    Pointi pt = mouse->GetCursorPos();
    Sizei renDims = render->GetRenderDimensions();
    if (pt.x < renDims.w && pt.y < renDims.h) {
        v10 = render->pActiveZBuffer[pt.x + pt.y * render->GetRenderDimensions().w] & 0xFFFF;
        if (v10) {
            if (SBPageCSpellsTextureList[v10]) {
                pX_coord =
                    pViewport->uViewportTL_X +
                    pIconPos[player->lastOpenedSpellbookPage]
                    [pSpellbookSpellIndices[player->lastOpenedSpellbookPage]
                    [v10]]
                .Xpos;
                pY_coord =
                    pViewport->uViewportTL_Y +
                    pIconPos[player->lastOpenedSpellbookPage]
                    [pSpellbookSpellIndices[player->lastOpenedSpellbookPage]
                    [v10]]
                .Ypos;

                render->DrawTextureNew(pX_coord / 640.0f, pY_coord / 480.0f,
                    SBPageCSpellsTextureList[v10]);
            }
        }
    }
}

void GUIWindow_Spellbook::Release() {
    OnCloseSpellBookPage();
    OnCloseSpellBook();

    GUIWindow::Release();
}

void LoadSpellbook(unsigned int spell_school) {
    byte_506550 = 0;

    // TODO(captainurist): encapsulate this enum arithmetic properly
    if (pParty->activeCharacter().uQuickSpell != SPELL_NONE &&
        (uint8_t)pParty->activeCharacter().uQuickSpell / 11 == spell_school)
        quick_spell_at_page =
            (uint8_t)pParty->activeCharacter().uQuickSpell - 11 * spell_school;
    else
        quick_spell_at_page = 0;

    for (unsigned int i = 1; i <= 11; ++i) {
        if (pParty->activeCharacter().spellbook.pChapters[spell_school].bIsSpellAvailable[i - 1] || engine->config->debug.AllMagic.value()) {
            char pContainer[20];
            sprintf(pContainer, "SB%sS%02d",
                    spellbook_texture_filename_suffices[spell_school],
                    pSpellbookSpellIndices[spell_school][i]);
            SBPageSSpellsTextureList[i] = assets->GetImage_Solid(pContainer);

            sprintf(pContainer, "SB%sC%02d",
                    spellbook_texture_filename_suffices[spell_school],
                    pSpellbookSpellIndices[spell_school][i]);
            SBPageCSpellsTextureList[i] = assets->GetImage_Solid(pContainer);
        }
    }
}

static void BookUI_Spellbook_DrawCurrentSchoolBackground() {
    int pTexID = 0;
    if (pParty->hasActiveCharacter()) {
        pTexID = pParty->activeCharacter().lastOpenedSpellbookPage;
    }
    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f,
                                ui_spellbook_school_backgrounds[pTexID]);

    render->DrawTextureNew(476 / 640.0f, 450 / 480.0f,
                                ui_spellbook_btn_quckspell);
    render->DrawTextureNew(561 / 640.0f, 450 / 480.0f,
                                ui_spellbook_btn_close);
}

void InitializeSpellBookTextures() {
    pAudioPlayer->playUISound(SOUND_openbook);

    ui_spellbook_btn_close = assets->GetImage_Solid("ib-m5-u");
    ui_spellbook_btn_close_click = assets->GetImage_Solid("ib-m5-d");
    ui_spellbook_btn_quckspell = assets->GetImage_Solid("ib-m6-u");
    ui_spellbook_btn_quckspell_click = assets->GetImage_Solid("ib-m6-d");

    static const char *texNames[9] = {"SBFB00", "SBAB00", "SBWB00",
                                      "SBEB00", "SBSB00", "SBMB00",
                                      "SBBB00", "SBLB00", "SBDB00"};

    for (unsigned int i = 0; i < 9; ++i) {
        ui_spellbook_school_backgrounds[i] = assets->GetImage_ColorKey(texNames[i]);
        ui_spellbook_school_tabs[i][0] =
            assets->GetImage_Alpha(fmt::format("tab{}a", i + 1));
        ui_spellbook_school_tabs[i][1] =
            assets->GetImage_Alpha(fmt::format("tab{}b", i + 1));
    }
}

void OnCloseSpellBook() {
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

    for (uint i = 0; i < 9; ++i) {
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

void OnCloseSpellBookPage() {
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
