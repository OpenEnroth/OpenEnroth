#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>

#define _CRT_SECURE_NO_WARNINGS

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Time.h"

#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Viewport.h"

#include "GUI/GUIButton.h"
#include "GUI/UI/UIRest.h"
#include "GUI/UI/UITransition.h"
#include "GUI/GUIFont.h"

#include "Media/Audio/AudioPlayer.h"

#include "Platform/Api.h"

Image *rest_ui_btn_4 = nullptr;
Image *rest_ui_btn_exit = nullptr;
Image *rest_ui_btn_3 = nullptr;
Image *rest_ui_btn_1 = nullptr;
Image *rest_ui_btn_2 = nullptr;
Image *rest_ui_restmain = nullptr;

Image *rest_ui_sky_frame_current = nullptr;
Image *rest_ui_hourglass_frame_current = nullptr;

void GUIWindow_RestWindow::Update() {
    __debugbreak();  // doesnt seems to get here, check stack trace & conditions
    GUIButton GUIButton2;  // [sp+28h] [bp-E0h]@133
    GUIButton2.uZ = 197;
    GUIButton2.uW = 197;
    GUIButton2.uX = 27;
    GUIButton2.uY = 161;
    GUIButton2.uWidth = 171;
    GUIButton2.uHeight = 37;
    GUIButton2.pParent = pButton_RestUI_WaitUntilDawn->pParent;
    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0);
    render->DrawTextureAlphaNew(uFrameX / 640.0f, uFrameY / 480.0f,
                                *((Image **)ptr_1C + 15));
    viewparams->bRedrawGameUI = 1;
    GUIButton2.DrawLabel(localization->GetString(183), pFontCreate, 0,
                         0);  // Rest & Heal 8 hrs / Отдых и лечение 8 часов
    GUIButton2.pParent = 0;
    Release();
}

void PrepareToLoadRestUI() {
    if (!_506F14_resting_stage) pAudioPlayer->StopChannels(-1, -1);
    if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) {
        if (pGUIWindow_CurrentMenu) {
            pGUIWindow_CurrentMenu->Release();
        }
        current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
        viewparams->bRedrawGameUI = true;
    }
    pEventTimer->Pause();
    if (_506F14_resting_stage != 2)
        new OnButtonClick2(518, 450, 0, 0, pBtn_Rest);
    _506F18_num_minutes_to_sleep = 0;
    _506F14_resting_stage = 0;
    uRestUI_FoodRequiredToRest = 2;
    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
        uRestUI_FoodRequiredToRest =
            pOutdoor->GetNumFoodRequiredToRestInCurrentPos(
                pParty->vPosition.x, pParty->vPosition.y, pParty->vPosition.z);
    if (PartyHasDragon()) {
        for (uint i = 0; i < 4; ++i) {
            if (pParty->pPlayers[i].classType == PLAYER_CLASS_WARLOCK)
                ++uRestUI_FoodRequiredToRest;
        }
    }
    if (CheckHiredNPCSpeciality(Porter)) --uRestUI_FoodRequiredToRest;
    if (CheckHiredNPCSpeciality(QuarterMaster)) uRestUI_FoodRequiredToRest -= 2;
    if (CheckHiredNPCSpeciality(Gypsy)) --uRestUI_FoodRequiredToRest;
    if (uRestUI_FoodRequiredToRest < 1) uRestUI_FoodRequiredToRest = 1;
    if ((pCurrentMapName == "d29.blv") &&
        _449B57_test_bit(pParty->_quest_bits,
                         PARTY_QUEST_HARMONDALE_REBUILT))  // d29 = Harmondale /
                                                           // Замок Хармондейл
        uRestUI_FoodRequiredToRest = 0;
}

//----- (0041F6C1) --------------------------------------------------------
GUIWindow_Rest::GUIWindow_Rest()
    : GUIWindow(WINDOW_Rest, 0, 0, window->GetWidth(), window->GetHeight(), 0) {
    PrepareToLoadRestUI();
    current_screen_type = CURRENT_SCREEN::SCREEN_REST;

    _507CD4_RestUI_hourglass_anim_controller = 0;
    rest_ui_restmain = assets->GetImage_Alpha("restmain");
    rest_ui_btn_1 = assets->GetImage_Alpha("restb1");
    rest_ui_btn_2 = assets->GetImage_Alpha("restb2");
    rest_ui_btn_3 = assets->GetImage_Alpha("restb3");
    rest_ui_btn_4 = assets->GetImage_Alpha("restb4");
    rest_ui_btn_exit = assets->GetImage_Alpha("restexit");

    OutdoorLocation::LoadActualSkyFrame();

    // auto wnd = new GUIWindow_Rest(0, 0, window->GetWidth(),
    // window->GetHeight());
    pButton_RestUI_Exit = CreateButton(280, 297, 154, 37, 1, 0, UIMSG_ExitRest,
                                       0, 0, "", {{rest_ui_btn_exit}});
    pButton_RestUI_Main = CreateButton(24, 154, 225, 37, 1, 0, UIMSG_Rest8Hour,
                                       0, 'R', "", {{rest_ui_btn_4}});
    pButton_RestUI_WaitUntilDawn =
        CreateButton(61, 232, 154, 33, 1, 0, UIMSG_AlreadyResting, 0, 'D', "",
                     {{rest_ui_btn_1}});
    pButton_RestUI_Wait1Hour = CreateButton(
        61, 264, 154, 33, 1, 0, UIMSG_Wait1Hour, 0, 'H', "", {{rest_ui_btn_2}});
    pButton_RestUI_Wait5Minutes =
        CreateButton(61, 296, 154, 33, 1, 0, UIMSG_Wait5Minutes, 0, 'M', "",
                     {{rest_ui_btn_3}});
}

//----- (0041FA01) --------------------------------------------------------
void GUIWindow_Rest::Update() {
    int live_characters;       // esi@1
    unsigned int v3;           // eax@15
    GUIButton tmp_button;      // [sp+8h] [bp-DCh]@19
    unsigned int am_pm_hours;  // [sp+D8h] [bp-Ch]@9

    live_characters = 0;
    for (int i = 1; i < 5; ++i)
        if (!pPlayers[i]->IsDead() && !pPlayers[i]->IsEradicated() &&
            pPlayers[i]->sHealth > 0)
            ++live_characters;

    if (live_characters) {
        render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f, rest_ui_restmain);
        am_pm_hours = pParty->uCurrentHour;
        dword_506F1C = pGUIWindow_CurrentMenu->pCurrentPosActiveItem;
        if ((signed int)pParty->uCurrentHour <= 12) {
            if (!am_pm_hours) am_pm_hours = 12;
        } else {
            am_pm_hours -= 12;
        }
        render->DrawTextureAlphaNew(16 / 640.0f, 26 / 480.0f,
                                    rest_ui_sky_frame_current);
        if (rest_ui_hourglass_frame_current) {
            rest_ui_hourglass_frame_current->Release();
            rest_ui_hourglass_frame_current = nullptr;
        }
        v3 = pEventTimer->uTimeElapsed +
             _507CD4_RestUI_hourglass_anim_controller;
        _507CD4_RestUI_hourglass_anim_controller += pEventTimer->uTimeElapsed;
        if ((unsigned int)_507CD4_RestUI_hourglass_anim_controller >= 512) {
            v3 = 0;
            _507CD4_RestUI_hourglass_anim_controller = 0;
        }
        hourglass_icon_idx =
            (int)floorf(((double)v3 / 512.0 * 120.0) + 0.5f) % 256 + 1;
        if (hourglass_icon_idx >= 120) hourglass_icon_idx = 1;

        {
            rest_ui_hourglass_frame_current = assets->GetImage_ColorKey(
                StringPrintf("hglas%03d", hourglass_icon_idx), 0x7FF);
            render->DrawTextureAlphaNew(267 / 640.0f, 159 / 480.0f,
                                        rest_ui_hourglass_frame_current);
        }

        tmp_button.uX = 24;
        tmp_button.uY = 154;
        tmp_button.uZ = 194;
        tmp_button.uW = 190;
        tmp_button.uWidth = 171;
        tmp_button.uHeight = 37;
        tmp_button.pParent = pButton_RestUI_WaitUntilDawn->pParent;
        tmp_button.DrawLabel(localization->GetString(183), pFontCreate,
                             Color16(10, 0, 0),
                             Color16(230, 214, 193));  //Отдых и лечение 8 часов
        tmp_button.pParent = 0;

        auto str1 = StringPrintf("\r408%d", uRestUI_FoodRequiredToRest);
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, 0, 164, Color16(10, 0, 0), str1, 0, 0, Color16(230, 214, 193));

        pButton_RestUI_WaitUntilDawn->DrawLabel(localization->GetString(237), pFontCreate, Color16(10, 0, 0), Color16(230, 214, 193));  // Ждать до рассвета
        pButton_RestUI_Wait1Hour->DrawLabel(localization->GetString(239), pFontCreate, Color16(10, 0, 0), Color16(230, 214, 193));  // Ждать 1 час
        pButton_RestUI_Wait5Minutes->DrawLabel(localization->GetString(238), pFontCreate, Color16(10, 0, 0), Color16(230, 214, 193));  // Ждать 5 минут
        pButton_RestUI_Exit->DrawLabel(localization->GetString(81), pFontCreate, Color16(10, 0, 0), Color16(230, 214, 193));  // Закончить отдыхать
        tmp_button.uX = 45;
        tmp_button.uY = 199;

        tmp_button.uZ = 229;
        tmp_button.uW = 228;

        tmp_button.uWidth = 185;
        tmp_button.uHeight = 30;

        tmp_button.pParent = pButton_RestUI_WaitUntilDawn->pParent;
        tmp_button.DrawLabel(localization->GetString(236), pFontCreate,
                             Color16(10, 0, 0),
                             Color16(230, 214, 193));  //Ждать без лечения
        tmp_button.pParent = 0;
        auto str2 = StringPrintf(
            "%d:%02d %s", am_pm_hours, pParty->uCurrentMinute,
            localization->GetAmPm(
                (pParty->uCurrentHour >= 12 && pParty->uCurrentHour < 24) ? 1
                                                                          : 0));
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, 368, 168,
                                         Color16(10, 0, 0), str2, 0, 0,
                                         Color16(230, 214, 193));
        auto str3 = StringPrintf("%s\r190%d", localization->GetString(56),
                                 pParty->uCurrentDayOfMonth + 1);  //День
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, 350, 190,
                                         Color16(10, 0, 0), str3, 0, 0,
                                         Color16(230, 214, 193));
        auto str4 = StringPrintf("%s\r190%d", localization->GetString(146),
                                 pParty->uCurrentMonth + 1);  //Месяц
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, 350, 222,
                                         Color16(10, 0, 0), str4, 0, 0,
                                         Color16(230, 214, 193));
        auto str5 = StringPrintf("%s\r190%d", localization->GetString(245),
                                 pParty->uCurrentYear);  //Год
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, 350, 254,
                                         Color16(10, 0, 0), str5, 0, 0,
                                         Color16(230, 214, 193));
        if (_506F14_resting_stage) Party::Sleep8Hours();
    } else {
        new OnCancel(pButton_RestUI_Exit->uX, pButton_RestUI_Exit->uY, 0, 0,
            (GUIButton *)pButton_RestUI_Exit,
            localization->GetString(81));  // "Exit Rest"
    }
}
