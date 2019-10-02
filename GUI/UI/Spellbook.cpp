#include "GUI/UI/Spellbook.h"

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Time.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"

#include "IO/Mouse.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"

#include "Media/Audio/AudioPlayer.h"

void InitializeSpellBookTextures();
void OnCloseSpellBookPage();
void OnCloseSpellBook();
void LoadSpellbook(unsigned int spell_school);
void BookUI_Spellbook_DrawCurrentSchoolBackground();

std::array<char *, 9> spellbook_texture_filename_suffices = {
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
    : GUIWindow(0, 0, window->GetWidth(), window->GetHeight(), 0) {
    current_screen_type = SCREEN_SPELL_BOOK;
    pEventTimer->Pause();

    InitializeSpellBookTextures();
    OpenSpellbook();

    pAudioPlayer->PlaySound(SOUND_48, 0, 0, -1, 0, 0);
    viewparams->field_48 = 1;
}

void GUIWindow_Spellbook::OpenSpellbookPage(int page) {
    OnCloseSpellBookPage();
    pPlayers[uActiveCharacter]->lastOpenedSpellbookPage = page;
    OpenSpellbook();
    pAudioPlayer->PlaySound((SoundID)(SOUND_TurnPageU + rand() % 2), 0, 0, -1, 0, 0);
}

void GUIWindow_Spellbook::OpenSpellbook() {
    Player *pPlayer;  // edi@1
    // GUIWindow *pWindow; // esi@1
    // unsigned int v3; // ebp@1
    int v4;  // eax@3
    /// GUIButton *result; // eax@25
    int a2;  // [sp+10h] [bp-8h]@1
    // int v7; // [sp+14h] [bp-4h]@1

    pPlayer = pPlayers[uActiveCharacter];
    // pWindow = this;
    LoadSpellbook(pPlayer->lastOpenedSpellbookPage);
    // v3 = 0;
    a2 = 0;

    PlayerSpellbookChapter *chapter =
        &pPlayer->spellbook.pChapters[pPlayer->lastOpenedSpellbookPage];
    for (uint i = 0; i < 11; ++i) {
        if (!chapter->bIsSpellAvailable[i] && !engine->config->debug_all_magic)
            continue;

        v4 = pPlayer->lastOpenedSpellbookPage;
        // v4 = (12 * pPlayer->lastOpenedSpellbookPage +
        // pSpellbookSpellIndices[pPlayer->lastOpenedSpellbookPage][i + 1]);
        CreateButton(pViewport->uViewportTL_X +
                         pIconPos[v4][pSpellbookSpellIndices[v4][i + 1]].Xpos,
                     pViewport->uViewportTL_Y +
                         pIconPos[v4][pSpellbookSpellIndices[v4][i + 1]]
                             .Ypos,  // dword_4E20D0
                     SBPageSSpellsTextureList[i + 1]->GetWidth(),
                     SBPageSSpellsTextureList[i + 1]->GetHeight(), 1, 79,
                     UIMSG_SelectSpell, i, 0, "");
        ++a2;
        // ++v3;
    }
    // while ( (signed int)v3 < 11 );

    CreateButton(0, 0, 0, 0, 1, 0, UIMSG_SpellBook_PressTab, 0, '\t', "");
    if (a2) _41D08F_set_keyboard_control_group(a2, 0, 0, 0);

    if (pPlayer->pActiveSkills[PLAYER_SKILL_FIRE] || engine->config->debug_all_magic)
        CreateButton(399, 10, 50, 36, 1, 0, UIMSG_OpenSpellbookPage, 0, 0,
                     localization->GetSpellSchoolName(0));
    if (pPlayer->pActiveSkills[PLAYER_SKILL_AIR] || engine->config->debug_all_magic)
        CreateButton(399, 46, 50, 36, 1, 0, UIMSG_OpenSpellbookPage, 1, 0,
                     localization->GetSpellSchoolName(1));
    if (pPlayer->pActiveSkills[PLAYER_SKILL_WATER] || engine->config->debug_all_magic)
        CreateButton(399, 83, 50, 36, 1, 0, UIMSG_OpenSpellbookPage, 2, 0,
                     localization->GetSpellSchoolName(2));
    if (pPlayer->pActiveSkills[PLAYER_SKILL_EARTH] || engine->config->debug_all_magic)
        CreateButton(399, 121, 50, 36, 1, 0, UIMSG_OpenSpellbookPage, 3, 0,
                     localization->GetSpellSchoolName(3));
    if (pPlayer->pActiveSkills[PLAYER_SKILL_SPIRIT] || engine->config->debug_all_magic)
        CreateButton(399, 158, 50, 36, 1, 0, UIMSG_OpenSpellbookPage, 4, 0,
                     localization->GetSpellSchoolName(5));
    if (pPlayer->pActiveSkills[PLAYER_SKILL_MIND] || engine->config->debug_all_magic)
        CreateButton(400, 196, 50, 36, 1, 0, UIMSG_OpenSpellbookPage, 5, 0,
                     localization->GetSpellSchoolName(4));
    if (pPlayer->pActiveSkills[PLAYER_SKILL_BODY] || engine->config->debug_all_magic)
        CreateButton(400, 234, 50, 36, 1, 0, UIMSG_OpenSpellbookPage, 6, 0,
                     localization->GetSpellSchoolName(6));
    if (pPlayer->pActiveSkills[PLAYER_SKILL_LIGHT] || engine->config->debug_all_magic)
        CreateButton(400, 271, 50, 36, 1, 0, UIMSG_OpenSpellbookPage, 7, 0,
                     localization->GetSpellSchoolName(7));
    if (pPlayer->pActiveSkills[PLAYER_SKILL_DARK] || engine->config->debug_all_magic)
        CreateButton(400, 307, 50, 36, 1, 0, UIMSG_OpenSpellbookPage, 8, 0,
                     localization->GetSpellSchoolName(8));

    CreateButton(476, 450, ui_spellbook_btn_quckspell->GetWidth(),
                 ui_spellbook_btn_quckspell->GetHeight(), 1, 78,
                 UIMSG_ClickInstallRemoveQuickSpellBtn, 0, 0, "");
    pBtn_InstallRemoveSpell = CreateButton(
        476, 450, 48, 32, 1, 78, UIMSG_ClickInstallRemoveQuickSpellBtn, 0, 0,
        "", {{ui_spellbook_btn_quckspell_click}});
    CreateButton(561, 450, ui_spellbook_btn_close->GetWidth(),
                 ui_spellbook_btn_close->GetHeight(), 1, 0, UIMSG_Escape, 0, 0,
                 localization->GetString(79));
    pBtn_CloseBook = CreateButton(561, 450, 48, 32, 1, 0, UIMSG_Escape, 0, 0,
                                  localization->GetString(79),
                                  {{ui_spellbook_btn_close_click}});
}

void GUIWindow_Spellbook::Update() {
    auto player = pPlayers[uActiveCharacter];

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

    render->ClearZBuffer(0, 479);

    for (unsigned int i = 0; i < 9; i++) {
        if (player->pActiveSkills[PLAYER_SKILL_FIRE + i] || engine->config->debug_all_magic) {
            auto pPageTexture = ui_spellbook_school_tabs[i][0];
            if (player->lastOpenedSpellbookPage == i) {
                pPageTexture = ui_spellbook_school_tabs[i][1];
                pX_coord = texture_tab_coord1[i][0];
                pY_coord = texture_tab_coord1[i][1];
            } else {
                pPageTexture = ui_spellbook_school_tabs[i][0];
                pX_coord = texture_tab_coord0[i][0];
                pY_coord = texture_tab_coord0[i][1];
            }
            render->DrawTextureAlphaNew(pX_coord / 640.0f, pY_coord / 480.0f,
                                        pPageTexture);

            PlayerSpellbookChapter *chapter =
                &player->spellbook.pChapters[player->lastOpenedSpellbookPage];
            for (unsigned int i = 0; i < 11; ++i) {
                if (chapter->bIsSpellAvailable[i] || engine->config->debug_all_magic) {
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

                            render->DrawTextureAlphaNew(
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
    }

    // if ((11 * player->lastOpenedSpellbookPage) || ((11 *
    // player->lastOpenedSpellbookPage) + 11))//??? maybe structure need fix
    //{

    // }

    Point pt = mouse->GetCursorPos();
    if (pt.x < 640 && pt.y < 480) {
        v10 = render->pActiveZBuffer[pt.x + pSRZBufferLineOffsets[pt.y]] & 0xFFFF;
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

                render->DrawTextureAlphaNew(pX_coord / 640.0f, pY_coord / 480.0f,
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
    if (pPlayers[uActiveCharacter]->uQuickSpell &&
        (unsigned __int8)pPlayers[uActiveCharacter]->uQuickSpell / 11 ==
            spell_school)
        quick_spell_at_page =
            (unsigned __int8)pPlayers[uActiveCharacter]->uQuickSpell -
            11 * spell_school;
    else
        quick_spell_at_page = 0;

    for (unsigned int i = 1; i <= 11; ++i) {
        if (pPlayers[uActiveCharacter]->spellbook.pChapters[spell_school].bIsSpellAvailable[i - 1] ||
            engine->config->debug_all_magic) {
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
    if (uActiveCharacter) {
        pTexID = pParty->pPlayers[uActiveCharacter - 1].lastOpenedSpellbookPage;
    }
    render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f,
                                ui_spellbook_school_backgrounds[pTexID]);

    render->DrawTextureAlphaNew(476 / 640.0f, 450 / 480.0f,
                                ui_spellbook_btn_quckspell);
    render->DrawTextureAlphaNew(561 / 640.0f, 450 / 480.0f,
                                ui_spellbook_btn_close);
}

void InitializeSpellBookTextures() {
    pAudioPlayer->StopChannels(-1, -1);
    pAudioPlayer->PlaySound(SOUND_openbook, 0, 0, -1, 0, 0);

    ui_spellbook_btn_close = assets->GetImage_Solid("ib-m5-u");
    ui_spellbook_btn_close_click = assets->GetImage_Solid("ib-m5-d");
    ui_spellbook_btn_quckspell = assets->GetImage_Solid("ib-m6-u");
    ui_spellbook_btn_quckspell_click = assets->GetImage_Solid("ib-m6-d");

    static const char *texNames[9] = {"SBFB00", "SBAB00", "SBWB00",
                                      "SBEB00", "SBSB00", "SBMB00",
                                      "SBBB00", "SBLB00", "SBDB00"};

    for (unsigned int i = 0; i < 9; ++i) {
        ui_spellbook_school_backgrounds[i] =
            assets->GetImage_ColorKey(texNames[i], 0x7FF);
        ui_spellbook_school_tabs[i][0] =
            assets->GetImage_Alpha(StringPrintf("tab%da", i + 1));
        ui_spellbook_school_tabs[i][1] =
            assets->GetImage_Alpha(StringPrintf("tab%db", i + 1));
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

    pAudioPlayer->PlaySound(SOUND_openbook, 0, 0, -1, 0, 0);
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
