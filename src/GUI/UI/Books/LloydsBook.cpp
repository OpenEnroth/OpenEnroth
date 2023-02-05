#include <string>

#include "Engine/Engine.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/SaveLoad.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/Books/LloydsBook.h"

std::array<int, 5> pLloydsBeaconsPreviewXs = {
    {61, 281, 61, 281, 171}};  // 004E249C
std::array<int, 5> pLloydsBeaconsPreviewYs = {{84, 84, 228, 228, 155}};
std::array<int, 5> pLloydsBeacons_SomeXs = {{59, 279, 59, 279, 169}};
std::array<int, 5> pLloydsBeacons_SomeYs = {{82, 82, 226, 226, 153}};

bool _506360_installing_beacon;  // 506360

Image *ui_book_lloyds_border = nullptr;
std::array<Image *, 2> ui_book_lloyds_backgrounds;

GUIWindow_LloydsBook::GUIWindow_LloydsBook() : GUIWindow_Book() {
    this->wData.val = WINDOW_LloydsBeacon;  // inherited from GUIWindow::GUIWindow
    BasicBookInitialization();

    _506360_installing_beacon = false;
    if (!ui_book_lloyds_border) {
        ui_book_lloyds_border = assets->GetImage_ColorKey("lb_bordr", colorTable.TealMask.C16());
    }

    ui_book_lloyds_backgrounds[0] = assets->GetImage_ColorKey("sbmap", colorTable.TealMask.C16());
    ui_book_lloyds_backgrounds[1] = assets->GetImage_ColorKey("sbmap", colorTable.TealMask.C16());
    ui_book_button1_on = assets->GetImage_Alpha("tab-an-6b");
    ui_book_button1_off = assets->GetImage_Alpha("tab-an-6a");

    pBtn_Book_1 = CreateButton({415, 13}, {39, 36}, 1, 0, UIMSG_LloydsBeacon_FlippingBtn, 0, InputAction::Invalid, localization->GetString(LSTR_SET_BEACON));
    pBtn_Book_2 = CreateButton({415, 48}, {39, 36}, 1, 0, UIMSG_LloydsBeacon_FlippingBtn, 1, InputAction::Invalid, localization->GetString(LSTR_RECALL_BEACON));

    int max_beacons = 1;
    int water_skill = pParty->pPlayers[_506348_current_lloyd_playerid]
                  .pActiveSkills[PLAYER_SKILL_WATER];

    if (water_skill & 0x100 || (water_skill & 0x80))
        max_beacons = 5;
    else if (water_skill & 0x40)
        max_beacons = 3;

    if (engine->config->debug.AllMagic.Get())
        max_beacons = 5;

    for (int i = 0; i < max_beacons; ++i) {
        CreateButton({pLloydsBeaconsPreviewXs[i], pLloydsBeaconsPreviewYs[i]}, {92, 68}, 1, UIMSG_HintBeaconSlot, UIMSG_InstallBeacon, i);
    }

    // purges expired beacons
    pParty->pPlayers[_506348_current_lloyd_playerid].CleanupBeacons();
}

void GUIWindow_LloydsBook::Update() {
    render->DrawTextureNew(
        471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);

    Player *pPlayer = &pParty->pPlayers[_506348_current_lloyd_playerid];
    render->DrawTextureNew(
        8 / 640.0f, 8 / 480.0f, ui_book_lloyds_backgrounds[bRecallingBeacon ? 1 : 0]);
    std::string pText = localization->GetString(LSTR_RECALL_BEACON);

    GUIWindow pWindow;
    pWindow.uFrameX = game_viewport_x;
    pWindow.uFrameY = game_viewport_y;
    pWindow.uFrameWidth = 428;
    pWindow.uFrameHeight = game_viewport_height;
    pWindow.uFrameZ = 435;
    pWindow.uFrameW = game_viewport_w;
    if (!bRecallingBeacon) {
        pText = localization->GetString(LSTR_SET_BEACON);
    }

    pWindow.DrawTitleText(pBook2Font, 0, 22, 0, pText, 3);
    if (bRecallingBeacon) {
        render->DrawTextureNew(pBtn_Book_1->uX / 640.0f,
                                    pBtn_Book_1->uY / 480.0f,
                                    ui_book_button1_on);
        render->DrawTextureNew(pBtn_Book_2->uX / 640.0f,
                                    pBtn_Book_2->uY / 480.0f,
                                    ui_book_button1_off);
    } else {
        render->DrawTextureNew(pBtn_Book_1->uX / 640.0f,
                                    pBtn_Book_1->uY / 480.0f,
                                    ui_book_button1_off);
        render->DrawTextureNew(pBtn_Book_2->uX / 640.0f,
                                    pBtn_Book_2->uY / 480.0f,
                                    ui_book_button1_on);
    }

    int uNumMaxBeacons = 1;
    if ((pPlayer->pActiveSkills[PLAYER_SKILL_WATER] & 0x100) ||
        (pPlayer->pActiveSkills[PLAYER_SKILL_WATER] & 0x80)) {
        uNumMaxBeacons = 5;
    } else if (pPlayer->pActiveSkills[PLAYER_SKILL_WATER] & 0x40) {
        uNumMaxBeacons = 3;
    }
    if (engine->config->debug.AllMagic.Get())
        uNumMaxBeacons = 5;

    if (uNumMaxBeacons > 0) {
        for (size_t BeaconID = 0; BeaconID < uNumMaxBeacons; BeaconID++) {
            if ((BeaconID >= pPlayer->vBeacons.size()) && bRecallingBeacon) {
                break;
            }

            pWindow.uFrameWidth = 92;
            pWindow.uFrameHeight = 68;
            pWindow.uFrameY = pLloydsBeaconsPreviewYs[BeaconID];
            pWindow.uFrameX = pLloydsBeaconsPreviewXs[BeaconID];
            pWindow.uFrameW = pWindow.uFrameY + 67;
            pWindow.uFrameZ = pLloydsBeaconsPreviewXs[BeaconID] + 91;

            render->DrawTextureNew(
                pLloydsBeacons_SomeXs[BeaconID] / 640.0f,
                pLloydsBeacons_SomeYs[BeaconID] / 480.0f,
                ui_book_lloyds_border);

            if (BeaconID < pPlayer->vBeacons.size()) {
                LloydBeacon &beacon = pPlayer->vBeacons[BeaconID];
                render->DrawTextureNew(
                    pLloydsBeaconsPreviewXs[BeaconID] / 640.0f,
                    pLloydsBeaconsPreviewYs[BeaconID] / 480.0f,
                    beacon.image);
                std::string Str = pMapStats->pInfos[pMapStats->sub_410D99_get_map_index(beacon.SaveFileID)].pName;
                unsigned int pTextHeight = pSpellFont->CalcTextHeight(Str, pWindow.uFrameWidth, 0);
                pWindow.uFrameY -= 6 + pTextHeight;
                pWindow.DrawTitleText(pSpellFont, 0, 0, 1, Str, 3);

                pWindow.uFrameY = pLloydsBeaconsPreviewYs[BeaconID];
                GameTime RemainingTime = beacon.uBeaconTime - pParty->GetPlayingTime();
                unsigned int pHours = RemainingTime.GetHoursOfDay();
                unsigned int pDays = RemainingTime.GetDays();
                std::string str;
                if (pDays > 1) {
                    str = StringPrintf("%lu %s", pDays + 1, localization->GetString(LSTR_DAYS));
                } else if (pHours + 1 <= 23) {
                    str = StringPrintf("%lu %s", pHours + 1, localization->GetString((pHours < 1) ? LSTR_HOUR : LSTR_HOURS));
                } else {
                    str = StringPrintf("%lu %s", pDays + 1, localization->GetString(LSTR_DAY_CAPITALIZED));
                }
                pWindow.uFrameY = pWindow.uFrameY + pWindow.uFrameHeight + 4;
                pWindow.DrawTitleText(pSpellFont, 0, 0, 1, str, 3);
            } else {
                unsigned int pTextHeight = pSpellFont->CalcTextHeight(
                    localization->GetString(LSTR_AVAILABLE), pWindow.uFrameWidth, 0);
                pWindow.DrawTitleText(pSpellFont, 0,
                    (int)pWindow.uFrameHeight / 2 - pTextHeight / 2, 1,
                    localization->GetString(LSTR_AVAILABLE), 3);
            }
        }
    }

    if (_506360_installing_beacon) {
        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_CloseAfterInstallBeacon, 0, 0);
    }
}
