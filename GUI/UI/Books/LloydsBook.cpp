#include "Engine/Engine.h"
#include "Engine/Localization.h"
#include "Engine/SaveLoad.h"
#include "Engine/LOD.h"
#include "Engine/Party.h"
#include "Engine/Graphics/IRender.h"

#include "GUI/GUIFont.h"
#include "GUI/UI/Books/LloydsBook.h"


std::array<unsigned int, 5> pLloydsBeaconsPreviewXs = { { 61, 281, 61, 281, 171 } }; // 004E249C
std::array<unsigned int, 5> pLloydsBeaconsPreviewYs = { { 84, 84, 228, 228, 155 } };
std::array<unsigned int, 5> pLloydsBeacons_SomeXs = { { 59, 279, 59, 279, 169 } };
std::array<unsigned int, 5> pLloydsBeacons_SomeYs = { { 82, 82, 226, 226, 153 } };


bool _506360_installing_beacon; // 506360

Image *ui_book_lloyds_border = nullptr;
std::array<Image *, 2> ui_book_lloyds_backgrounds;

GUIWindow_LloydsBook::GUIWindow_LloydsBook() :
    GUIWindow_Book()
{
    this->ptr_1C = (void *)WINDOW_LloydsBeacon; // inherited from GUIWindow::GUIWindow
    BasicBookInitialization();

// ----------------------------------------------
// 00411BFC GUIWindow::InitializeBookView -- part
    _506360_installing_beacon = false;
    if (!ui_book_lloyds_border)
        ui_book_lloyds_border = assets->GetImage_16BitColorKey("lb_bordr", 0x7FF);

    ui_book_lloyds_backgrounds[0] = assets->GetImage_16BitColorKey("sbmap", 0x7FF);
    ui_book_lloyds_backgrounds[1] = assets->GetImage_16BitColorKey("sbmap", 0x7FF);
    ui_book_button1_on = assets->GetImage_16BitAlpha("tab-an-6b");
    ui_book_button1_off = assets->GetImage_16BitAlpha("tab-an-6a");

    pBtn_Book_1 = CreateButton(415, 13, 39, 36, 1, 0, UIMSG_LloydsBeacon_FlippingBtn, 0, 0, localization->GetString(375), 0); // Set Beacon
    pBtn_Book_2 = CreateButton(415, 48, 39, 36, 1, 0, UIMSG_LloydsBeacon_FlippingBtn, 1, 0, localization->GetString(523), 0); // Recall Beacon

    int max_beacons = 1;
    int v18 = pParty->pPlayers[_506348_current_lloyd_playerid].pActiveSkills[PLAYER_SKILL_WATER];
    if (v18 & 0x100 || (v18 & 0x80))
        max_beacons = 5;
    else if (v18 & 0x40)
        max_beacons = 3;

    for (int i = 0; i < max_beacons; ++i)
    {
        CreateButton(
            pLloydsBeaconsPreviewXs[i], pLloydsBeaconsPreviewYs[i],
            92, 68, 1, 180, UIMSG_InstallBeacon, i, 0, "", 0
        );
    }

    for (int i = 0; i < 5; ++i)
    {
        if (pParty->pPlayers[_506348_current_lloyd_playerid].pInstalledBeacons[i].uBeaconTime >= pParty->GetPlayingTime())
            LoadThumbnailLloydTexture(i, _506348_current_lloyd_playerid + 1);
        else
            memset(&pParty->pPlayers[_506348_current_lloyd_playerid].pInstalledBeacons[i], 0, sizeof(LloydBeacon));
    }
}


void GUIWindow_LloydsBook::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
// {
//     BookUI_Draw((WindowType)(int)ptr_1C);
// }

// ----- (00413CC6) --------------------------------------------------------
// void BookUI_Draw(WindowType book) --- part
// {
    render->DrawTextureAlphaNew(471/640.0f, 445/480.0f, ui_exit_cancel_button_background);
//     DrawLloydBeaconsScreen();

// ----- (00410DEC) --------------------------------------------------------
// unsigned int DrawLloydBeaconsScreen()
// {
    Player *pPlayer; // esi@1
    const char *pText; // eax@1
    int pTextHeight; // eax@14
    GameTime RemainingTime; // kr08_8@14
    unsigned int pHours; // esi@14
    unsigned int pDays; // eax@14
    const char *pSelectionText; // eax@19
    GUIWindow pWindow; // [sp+Ch] [bp-7Ch]@1
    char *Str; // [sp+74h] [bp-14h]@14
    int BeaconID; // [sp+78h] [bp-10h]@11
    int uNumMaxBeacons; // [sp+84h] [bp-4h]@6

    pPlayer = &pParty->pPlayers[_506348_current_lloyd_playerid];
    render->DrawTextureAlphaNew(8/640.0f, 8/480.0f, ui_book_lloyds_backgrounds[bRecallingBeacon ? 1 : 0]);
    pText = localization->GetString(523);     // Recall Beacon
    pWindow.uFrameX = game_viewport_x;
    pWindow.uFrameY = game_viewport_y;
    pWindow.uFrameWidth = 428;
    pWindow.uFrameHeight = game_viewport_height;
    pWindow.uFrameZ = 435;
    pWindow.uFrameW = game_viewport_w;
    if (!bRecallingBeacon)
        pText = localization->GetString(375);   // Set Beacon

    pWindow.DrawTitleText(pBook2Font, 0, 22, 0, pText, 3);
    if (bRecallingBeacon)
    {
        render->DrawTextureAlphaNew(pBtn_Book_1->uX/640.0f, pBtn_Book_1->uY/480.0f, ui_book_button1_on);
        render->DrawTextureAlphaNew(pBtn_Book_2->uX/640.0f, pBtn_Book_2->uY/480.0f, ui_book_button1_off);
    }
    else
    {
        render->DrawTextureAlphaNew(pBtn_Book_1->uX/640.0f, pBtn_Book_1->uY/480.0f, ui_book_button1_off);
        render->DrawTextureAlphaNew(pBtn_Book_2->uX/640.0f, pBtn_Book_2->uY/480.0f, ui_book_button1_on);
    }
    uNumMaxBeacons = 1;
    if ((pPlayer->pActiveSkills[PLAYER_SKILL_WATER] & 0x100) || (pPlayer->pActiveSkills[PLAYER_SKILL_WATER] & 0x80))
    {
        uNumMaxBeacons = 5;
    }
    else if (pPlayer->pActiveSkills[PLAYER_SKILL_WATER] & 0x40)
    {
        uNumMaxBeacons = 3;
    }

    if (uNumMaxBeacons > 0)
    {
        for (BeaconID = 0; BeaconID < uNumMaxBeacons; BeaconID++)
        {
            pWindow.uFrameWidth = 92;
            pWindow.uFrameHeight = 68;
            pWindow.uFrameY = pLloydsBeaconsPreviewYs[BeaconID];
            pWindow.uFrameX = pLloydsBeaconsPreviewXs[BeaconID];
            pWindow.uFrameW = pWindow.uFrameY + 67;
            pWindow.uFrameZ = pLloydsBeaconsPreviewXs[BeaconID] + 91;
            //if ( pSavegameThumbnails[BeaconID].pPixels != 0 )
            if (pPlayer->pInstalledBeacons[BeaconID].SaveFileID != 0)
            {
                render->DrawTextureAlphaNew(pLloydsBeacons_SomeXs[BeaconID]/640.0f, pLloydsBeacons_SomeYs[BeaconID]/480.0f, ui_book_lloyds_border);
                render->DrawTextureNew(pLloydsBeaconsPreviewXs[BeaconID]/640.0f, pLloydsBeaconsPreviewYs[BeaconID]/480.0f, pSavegameThumbnails[BeaconID]);
                Str = pMapStats->pInfos[pMapStats->sub_410D99_get_map_index(pPlayer->pInstalledBeacons[BeaconID].SaveFileID)].pName;
                pTextHeight = pSpellFont->CalcTextHeight(Str, &pWindow, 0);
                pWindow.uFrameY += -6 - pTextHeight;
                pWindow.DrawTitleText(pSpellFont, 0, 0, 1, Str, 3);
                RemainingTime = pPlayer->pInstalledBeacons[BeaconID].uBeaconTime - pParty->GetPlayingTime();
                pHours = RemainingTime.GetHoursOfDay();
                pDays = RemainingTime.GetDays();
                if (pDays)
                {
                    auto str = StringPrintf("%lu %s", pDays + 1, localization->GetString(57));//days
                    pWindow.uFrameY = pWindow.uFrameY + pWindow.uFrameHeight + 4;
                    pWindow.DrawTitleText(pSpellFont, 0, 0, 1, str, 3);
                    continue;
                }
                else
                {
                    if (pHours + 1 <= 23)
                    {
                        if (pHours < 1)
                            pSelectionText = localization->GetString(109);// Hour
                        else
                            pSelectionText = localization->GetString(110);// Hours
                        auto str = StringPrintf("%lu %s", pHours + 1, pSelectionText);
                        pWindow.uFrameY = pWindow.uFrameY + pWindow.uFrameHeight + 4;
                        pWindow.DrawTitleText(pSpellFont, 0, 0, 1, str, 3);
                        continue;
                    }
                }
                auto str = StringPrintf("%lu %s", pDays + 1, localization->GetString(56)); //Day
                pWindow.uFrameY = pWindow.uFrameY + pWindow.uFrameHeight + 4;
                pWindow.DrawTitleText(pSpellFont, 0, 0, 1, str, 3);
                continue;
            }
            if (!bRecallingBeacon)
            {
                render->DrawTextureAlphaNew(pLloydsBeacons_SomeXs[BeaconID]/640.0f, pLloydsBeacons_SomeYs[BeaconID]/480.0f, ui_book_lloyds_border);
                pTextHeight = pSpellFont->CalcTextHeight(localization->GetString(19), &pWindow, 0);
                pWindow.DrawTitleText(pSpellFont, 0, (signed int)pWindow.uFrameHeight / 2 - pTextHeight / 2, 1, localization->GetString(19), 3);//Доступно
            }
        }
    }

    if (_506360_installing_beacon)
        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_CloseAfterInstallBeacon, 0, 0);
}