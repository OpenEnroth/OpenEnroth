#include <string>

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Time.h"

#include "GUI/GUIButton.h"
#include "GUI/UI/UIRest.h"
#include "GUI/GUIFont.h"

#include "Media/Audio/AudioPlayer.h"

Image *rest_ui_btn_4 = nullptr;
Image *rest_ui_btn_exit = nullptr;
Image *rest_ui_btn_3 = nullptr;
Image *rest_ui_btn_1 = nullptr;
Image *rest_ui_btn_2 = nullptr;
Image *rest_ui_restmain = nullptr;

Image *rest_ui_sky_frame_current = nullptr;
Image *rest_ui_hourglass_frame_current = nullptr;

int foodRequiredToRest;
GameTime remainingRestTime;
REST_TYPE currentRestType;

// GUIWindow_RestWindow is unused
#if 0
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
    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    render->DrawTextureNew(uFrameX / 640.0f, uFrameY / 480.0f, *(static_cast<Image **>(wData.ptr) + 15));
    GUIButton2.DrawLabel(localization->GetString(LSTR_REST_AND_HEAL_8_HOURS), pFontCreate, 0, 0);
    GUIButton2.pParent = 0;
    Release();
}
#endif

static void prepareToLoadRestUI() {
    if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME) {
        if (pGUIWindow_CurrentMenu) {
            pGUIWindow_CurrentMenu->Release();
        }
        current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
    }
    pEventTimer->Pause();
    if (currentRestType != REST_HEAL) {
        new OnButtonClick2({518, 450}, {0, 0}, pBtn_Rest);
    }
    remainingRestTime = GameTime();
    currentRestType = REST_NONE;
}

static void calculateRequiredFood() {
    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
        foodRequiredToRest = pOutdoor->getNumFoodRequiredToRestInCurrentPos(pParty->vPosition);
    } else {
        foodRequiredToRest = 2;
    }

    if (PartyHasDragon()) {
        for (Player &player : pParty->pPlayers) {
            if (player.classType == PLAYER_CLASS_WARLOCK) {
                ++foodRequiredToRest;
            }
        }
    }

    if (CheckHiredNPCSpeciality(Porter)) {
        --foodRequiredToRest;
    }
    if (CheckHiredNPCSpeciality(QuarterMaster)) {
        foodRequiredToRest -= 2;
    }
    if (CheckHiredNPCSpeciality(Gypsy)) {
        --foodRequiredToRest;
    }
    if (foodRequiredToRest < 1) {
        foodRequiredToRest = 1;
    }
    if (pCurrentMapName == "d29.blv" && pParty->_questBits[QBIT_HARMONDALE_REBUILT]) {
        foodRequiredToRest = 0;
    }
}

GUIWindow_Rest::GUIWindow_Rest()
    : GUIWindow(WINDOW_Rest, {0, 0}, render->GetRenderDimensions(), 0) {
    prepareToLoadRestUI();
    calculateRequiredFood();

    current_screen_type = CURRENT_SCREEN::SCREEN_REST;

    hourglassLoopTimer = 0;
    rest_ui_restmain = assets->GetImage_Alpha("restmain");
    rest_ui_btn_1 = assets->GetImage_Alpha("restb1");
    rest_ui_btn_2 = assets->GetImage_Alpha("restb2");
    rest_ui_btn_3 = assets->GetImage_Alpha("restb3");
    rest_ui_btn_4 = assets->GetImage_Alpha("restb4");
    rest_ui_btn_exit = assets->GetImage_Alpha("restexit");

    OutdoorLocation::LoadActualSkyFrame();

    // auto wnd = new GUIWindow_Rest(0, 0, window->GetWidth(),
    // window->GetHeight());
    pButton_RestUI_Exit = CreateButton({280, 297}, {154, 37}, 1, 0, UIMSG_ExitRest, 0, InputAction::Invalid, "", {rest_ui_btn_exit});
    pButton_RestUI_Main = CreateButton({24, 154}, {225, 37}, 1, 0, UIMSG_Rest8Hour, 0, InputAction::Rest8Hours, "", {rest_ui_btn_4});
    pButton_RestUI_WaitUntilDawn = CreateButton({61, 232}, {154, 33}, 1, 0, UIMSG_WaitTillDawn, 0, InputAction::WaitTillDawn, "", {rest_ui_btn_1});
    pButton_RestUI_Wait1Hour = CreateButton({61, 264}, {154, 33}, 1, 0, UIMSG_Wait1Hour, 0, InputAction::WaitHour, "", {rest_ui_btn_2});
    pButton_RestUI_Wait5Minutes = CreateButton({61, 296}, {154, 33}, 1, 0, UIMSG_Wait5Minutes, 0, InputAction::Wait5Minutes, "", {rest_ui_btn_3});
}

void GUIWindow_Rest::Update() {
    GUIButton tmp_button;

    int liveCharacters = 0;
    for (Player &player : pParty->pPlayers) {
        if (!player.IsDead() && !player.IsEradicated() && player.health > 0) {
            ++liveCharacters;
        }
    }

    if (liveCharacters) {
        render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, rest_ui_restmain);
        int am_pm_hours = pParty->uCurrentHour;
        //dword_506F1C = pGUIWindow_CurrentMenu->pCurrentPosActiveItem;
        if (pParty->uCurrentHour <= 12) {
            if (!am_pm_hours) {
                am_pm_hours = 12;
            }
        } else {
            am_pm_hours -= 12;
        }
        render->DrawTextureNew(16 / 640.0f, 26 / 480.0f, rest_ui_sky_frame_current);
        if (rest_ui_hourglass_frame_current) {
            rest_ui_hourglass_frame_current->Release();
            rest_ui_hourglass_frame_current = nullptr;
        }

        hourglassLoopTimer += pEventTimer->uTimeElapsed;
        if (hourglassLoopTimer >= (Timer::Second * 4)) {
            hourglassLoopTimer = 0;
        }

        int hourglass_icon_idx = (int)floorf(((double)hourglassLoopTimer / 512.0 * 120.0) + 0.5f) % 256 + 1;
        if (hourglass_icon_idx >= 120) {
            hourglass_icon_idx = 1;
        }

        rest_ui_hourglass_frame_current = assets->GetImage_ColorKey(fmt::format("hglas{:03}", hourglass_icon_idx));
        render->DrawTextureNew(267 / 640.0f, 159 / 480.0f, rest_ui_hourglass_frame_current);

        tmp_button.uX = 24;
        tmp_button.uY = 154;
        tmp_button.uZ = 194;
        tmp_button.uW = 190;
        tmp_button.uWidth = 171;
        tmp_button.uHeight = 37;
        tmp_button.pParent = pButton_RestUI_WaitUntilDawn->pParent;
        tmp_button.DrawLabel(localization->GetString(LSTR_REST_AND_HEAL_8_HOURS), pFontCreate, colorTable.Diesel.c16(), colorTable.StarkWhite.c16());
        tmp_button.pParent = 0;

        auto str1 = fmt::format("\r408{}", foodRequiredToRest);
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, {0, 164}, colorTable.Diesel.c16(), str1, 0, 0, colorTable.StarkWhite.c16());

        pButton_RestUI_WaitUntilDawn->DrawLabel(localization->GetString(LSTR_WAIT_UNTIL_DAWN), pFontCreate, colorTable.Diesel.c16(), colorTable.StarkWhite.c16());
        pButton_RestUI_Wait1Hour->DrawLabel(localization->GetString(LSTR_WAIT_1_HOUR), pFontCreate, colorTable.Diesel.c16(), colorTable.StarkWhite.c16());
        pButton_RestUI_Wait5Minutes->DrawLabel(localization->GetString(LSTR_WAIT_5_MINUTES), pFontCreate, colorTable.Diesel.c16(), colorTable.StarkWhite.c16());
        pButton_RestUI_Exit->DrawLabel(localization->GetString(LSTR_EXIT_REST), pFontCreate, colorTable.Diesel.c16(), colorTable.StarkWhite.c16());
        tmp_button.uX = 45;
        tmp_button.uY = 199;

        tmp_button.uZ = 229;
        tmp_button.uW = 228;

        tmp_button.uWidth = 185;
        tmp_button.uHeight = 30;

        tmp_button.pParent = pButton_RestUI_WaitUntilDawn->pParent;
        tmp_button.DrawLabel(localization->GetString(LSTR_WAIT_WITHOUT_HEALING), pFontCreate, colorTable.Diesel.c16(), colorTable.StarkWhite.c16());
        tmp_button.pParent = 0;
        std::string str2 = fmt::format("{}:{:02} {}", am_pm_hours, pParty->uCurrentMinute, localization->GetAmPm((pParty->uCurrentHour >= 12 && pParty->uCurrentHour < 24) ? 1 : 0));
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, {368, 168}, colorTable.Diesel.c16(), str2, 0, 0, colorTable.StarkWhite.c16());
        std::string str3 = fmt::format("{}\r190{}", localization->GetString(LSTR_DAY_CAPITALIZED), pParty->uCurrentDayOfMonth + 1);
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, {350, 190}, colorTable.Diesel.c16(), str3, 0, 0, colorTable.StarkWhite.c16());
        std::string str4 = fmt::format("{}\r190{}", localization->GetString(LSTR_MONTH), pParty->uCurrentMonth + 1);
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, {350, 222}, colorTable.Diesel.c16(), str4, 0, 0, colorTable.StarkWhite.c16());
        std::string str5 = fmt::format("{}\r190{}", localization->GetString(LSTR_YEAR), pParty->uCurrentYear);
        pGUIWindow_CurrentMenu->DrawText(pFontCreate, {350, 254}, colorTable.Diesel.c16(), str5, 0, 0, colorTable.StarkWhite.c16());
        if (currentRestType != REST_NONE) {
            Party::restOneFrame();
        }
    } else {
        new OnCancel({pButton_RestUI_Exit->uX, pButton_RestUI_Exit->uY}, {0, 0}, pButton_RestUI_Exit, localization->GetString(LSTR_EXIT_REST));
    }
}
