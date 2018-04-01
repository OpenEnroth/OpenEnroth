#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"
#include "Engine/Localization.h"
#include "Engine/LOD.h"
#include "Engine/MMT.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Tables/IconFrameTable.h"

#include "IO/Mouse.h"
#include "IO/Keyboard.h"

#include "GUI/GUIFont.h"
#include "GUI/UI/UIMainMenu.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIPartyCreation.h"

#include "Media/Audio/AudioPlayer.h"

#include "Game/Game.h"
#include "Game/MainMenu.h"



GUIButton *pMainMenu_BtnExit = nullptr;
GUIButton *pMainMenu_BtnCredits = nullptr;
GUIButton *pMainMenu_BtnLoad = nullptr;
GUIButton *pMainMenu_BtnNew = nullptr;

Image *ui_mainmenu_new = nullptr;
Image *ui_mainmenu_load = nullptr;
Image *ui_mainmenu_credits = nullptr;
Image *ui_mainmenu_exit = nullptr;

GUIWindow_MainMenu::GUIWindow_MainMenu() :
  GUIWindow(0, 0, window->GetWidth(), window->GetHeight(), 0)
{
  ui_mainmenu_new = assets->GetImage_16BitColorKey("title_new", 0x7FF);
  ui_mainmenu_load = assets->GetImage_16BitColorKey("title_load", 0x7FF);
  ui_mainmenu_credits = assets->GetImage_16BitColorKey("title_cred", 0x7FF);
  ui_mainmenu_exit = assets->GetImage_16BitColorKey("title_exit", 0x7FF);

  pMainMenu_BtnNew = CreateButton(495, 172, ui_mainmenu_new->GetWidth(), ui_mainmenu_new->GetHeight(), 1, 0, UIMSG_MainMenu_ShowPartyCreationWnd, 0, 'N', "", { {ui_mainmenu_new} });
  pMainMenu_BtnLoad = CreateButton(495, 227, ui_mainmenu_load->GetWidth(), ui_mainmenu_load->GetHeight(), 1, 0, UIMSG_MainMenu_ShowLoadWindow, 1, 'L', "", { {ui_mainmenu_load} });
  pMainMenu_BtnCredits = CreateButton(495, 282, ui_mainmenu_credits->GetWidth(), ui_mainmenu_credits->GetHeight(), 1, 0, UIMSG_ShowCredits, 2, 'C', "", { {ui_mainmenu_credits} });
  pMainMenu_BtnExit = CreateButton(495, 337, ui_mainmenu_exit->GetWidth(), ui_mainmenu_exit->GetHeight(), 1, 0, UIMSG_ExitToWindows, 3, 0, "", { {ui_mainmenu_exit} });
}

void GUIWindow_MainMenu::Update() {
    Point pt = pMouse->GetCursorPos();
    GUIWindow *pWindow = this;//pWindow_MainMenu;
    //if (GetCurrentMenuID() == MENU_SAVELOAD)
    //    pWindow = pGUIWindow_CurrentMenu;

    if (GetCurrentMenuID() == MENU_MAIN)
    {
        Image *pTexture = nullptr;
        if (!pModalWindow)// ???
        {
            auto pButton = pWindow->pControlsHead;
            for (pButton = pWindow->pControlsHead; pButton; pButton = pButton->pNext)
            {
                if (pt.x >= (signed int)pButton->uX && pt.x <= (signed int)pButton->uZ
                    && pt.y >= (signed int)pButton->uY && pt.y <= (signed int)pButton->uW
                    && pWindow == pWindow_MainMenu)
                {
                    auto pControlParam = pButton->msg_param;
                    int pY = 0;
                    switch (pControlParam) // backlight for buttons
                    {
                    case 0:
                        pTexture = assets->GetImage_16BitColorKey("title_new", 0x7FF);
                        pY = 172;
                        break;
                    case 1:
                        pTexture = assets->GetImage_16BitColorKey("title_load", 0x7FF);
                        pY = 227;
                        break;
                    case 2:
                        pTexture = assets->GetImage_16BitColorKey("title_cred", 0x7FF);
                        pY = 282;
                        break;
                    case 3:
                        pTexture = assets->GetImage_16BitColorKey("title_exit", 0x7FF);
                        pY = 337;
                        break;
                    }
                    render->DrawTextureAlphaNew(495/640.0f, pY/480.0f, pTexture);
                }
            }
        }
    }
}


//----- (0041B578) --------------------------------------------------------
void MainMenuUI_LoadFontsAndSomeStuff()
{
  //pIcons_LOD->SetupPalettes(render->uTargetRBits, render->uTargetGBits, render->uTargetBBits);
  pIcons_LOD->SetupPalettes(5, 6, 5);
  //pPaletteManager->SetColorChannelInfo(render->uTargetRBits, render->uTargetGBits, render->uTargetBBits);
  pPaletteManager->SetColorChannelInfo(5, 6, 5);
  pPaletteManager->RecalculateAll();

  for (uint i = 0; i < window->GetHeight(); ++i)
    pSRZBufferLineOffsets[i] = window->GetWidth() * i;

  pFontArrus = GUIFont::LoadFont("arrus.fnt", "FONTPAL", nullptr);
  pFontLucida = GUIFont::LoadFont("lucida.fnt", "FONTPAL", nullptr);
  pFontCreate = GUIFont::LoadFont("create.fnt", "FONTPAL", nullptr);
  pFontSmallnum = GUIFont::LoadFont("smallnum.fnt", "FONTPAL", nullptr);
  pFontComic = GUIFont::LoadFont("comic.fnt", "FONTPAL", nullptr);
}

//----- (004415C5) --------------------------------------------------------
static void LoadPartyBuffIcons()
{
    for (uint i = 0; i < 14; ++i)
    {
        party_buff_icons[i] = assets->GetImage_16BitColorKey(StringPrintf("isn-%02d", i + 1), 0x7FF);
    }

    uIconIdx_FlySpell = pIconsFrameTable->FindIcon("spell21");
    uIconIdx_WaterWalk = pIconsFrameTable->FindIcon("spell27");
}

//----- (0041B690) --------------------------------------------------------
void MainMenuUI_Create()
{
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("wizeyeC"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("wizeyeB"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("wizeyeA"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("torchC"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("torchB"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("torchA"));

    game_ui_minimap_dirs[0] = assets->GetImage_16BitAlpha("MAPDIR1");
    game_ui_minimap_dirs[1] = assets->GetImage_16BitAlpha("MAPDIR2");
    game_ui_minimap_dirs[2] = assets->GetImage_16BitAlpha("MAPDIR3");
    game_ui_minimap_dirs[3] = assets->GetImage_16BitAlpha("MAPDIR4");
    game_ui_minimap_dirs[4] = assets->GetImage_16BitAlpha("MAPDIR5");
    game_ui_minimap_dirs[5] = assets->GetImage_16BitAlpha("MAPDIR6");
    game_ui_minimap_dirs[6] = assets->GetImage_16BitAlpha("MAPDIR7");
    game_ui_minimap_dirs[7] = assets->GetImage_16BitAlpha("MAPDIR8");

    game_ui_bar_blue = assets->GetImage_16BitColorKey("ib-statB", 0x7FF);
    game_ui_bar_green = assets->GetImage_16BitColorKey("ib-statG", 0x7FF);
    game_ui_bar_yellow = assets->GetImage_16BitColorKey("ib-statY", 0x7FF);
    game_ui_bar_red = assets->GetImage_16BitColorKey("ib-statR", 0x7FF);
    game_ui_monster_hp_background = assets->GetImage_16BitColorKey("mhp_bg", 0x7FF);
    game_ui_monster_hp_border_left = assets->GetImage_16BitColorKey("mhp_capl", 0x7FF);
    game_ui_monster_hp_border_right = assets->GetImage_16BitColorKey("mhp_capr", 0x7FF);
    game_ui_monster_hp_green = assets->GetImage_16BitColorKey("mhp_grn", 0x7FF);
    game_ui_monster_hp_red = assets->GetImage_16BitColorKey("mhp_red", 0x7FF);
    game_ui_monster_hp_yellow = assets->GetImage_16BitColorKey("mhp_yel", 0x7FF);
    ui_leather_mm7 = assets->GetImage_16Bit("LEATHER");
    ui_leather_mm6 = assets->GetImage_16Bit("ibground");
    dialogue_ui_x_x_u = assets->GetImage_16BitColorKey("x_x_u", 0x7FF);
    ui_buttdesc2 = assets->GetImage_16BitAlpha("BUTTESC2");
    dialogue_ui_x_ok_u = assets->GetImage_16BitColorKey("x_ok_u", 0x7FF);
    ui_buttyes2 = assets->GetImage_16BitAlpha("BUTTYES2");

    pPrimaryWindow = new GUIWindow(0, 0, window->GetWidth(), window->GetHeight(), 0);
    pPrimaryWindow->CreateButton(7, 8, 460, 343, 1, 0, UIMSG_MouseLeftClickInGame, 0, 0, "");

    pPrimaryWindow->CreateButton(61, 424, 31, 80, 2, 94, UIMSG_SelectCharacter, 1, '1', "");//buttons for portraits
    pPrimaryWindow->CreateButton(177, 424, 31, 80, 2, 94, UIMSG_SelectCharacter, 2, '2', "");
    pPrimaryWindow->CreateButton(292, 424, 31, 40, 2, 94, UIMSG_SelectCharacter, 3, '3', "");
    pPrimaryWindow->CreateButton(407, 424, 31, 40, 2, 94, UIMSG_SelectCharacter, 4, '4', "");

    pPrimaryWindow->CreateButton(24, 404, 5, 49, 1, 93, UIMSG_0, 1, 0, "");//buttons for HP
    pPrimaryWindow->CreateButton(139, 404, 5, 49, 1, 93, UIMSG_0, 2, 0, "");
    pPrimaryWindow->CreateButton(255, 404, 5, 49, 1, 93, UIMSG_0, 3, 0, "");
    pPrimaryWindow->CreateButton(370, 404, 5, 49, 1, 93, UIMSG_0, 4, 0, "");

    pPrimaryWindow->CreateButton(97, 404, 5, 49, 1, 93, UIMSG_0, 1, 0, "");//buttons for SP
    pPrimaryWindow->CreateButton(212, 404, 5, 49, 1, 93, UIMSG_0, 2, 0, "");
    pPrimaryWindow->CreateButton(328, 404, 5, 49, 1, 93, UIMSG_0, 3, 0, "");
    pPrimaryWindow->CreateButton(443, 404, 5, 49, 1, 93, UIMSG_0, 4, 0, "");

    game_ui_tome_quests = assets->GetImage_16BitColorKey("ib-td1-A", 0x7FF);
    pBtn_Quests = pPrimaryWindow->CreateButton(
        491, 353,
        game_ui_tome_quests->GetWidth(),
        game_ui_tome_quests->GetHeight(),
        1, 0, UIMSG_OpenQuestBook, 0, pKeyActionMap->GetActionVKey(INPUT_Quest),
      localization->GetString(174), { {game_ui_tome_quests} }); //Quests

    game_ui_tome_autonotes = assets->GetImage_16BitColorKey("ib-td2-A", 0x7FF);
    pBtn_Autonotes = pPrimaryWindow->CreateButton(
        527, 353,
        game_ui_tome_autonotes->GetWidth(),
        game_ui_tome_autonotes->GetHeight(),
        1, 0, UIMSG_OpenAutonotes, 0, pKeyActionMap->GetActionVKey(INPUT_Autonotes),
      localization->GetString(154), { {game_ui_tome_autonotes} });//Autonotes

    game_ui_tome_maps = assets->GetImage_16BitColorKey("ib-td3-A", 0x7FF);
    pBtn_Maps = pPrimaryWindow->CreateButton(
        546, 353,
        game_ui_tome_maps->GetWidth(),
        game_ui_tome_maps->GetHeight(),
        1, 0, UIMSG_OpenMapBook, 0, pKeyActionMap->GetActionVKey(INPUT_Mapbook),
      localization->GetString(139), { {game_ui_tome_maps} }); //Maps

    game_ui_tome_calendar = assets->GetImage_16BitColorKey("ib-td4-A", 0x7FF);
    pBtn_Calendar = pPrimaryWindow->CreateButton(
        570, 353,
        game_ui_tome_calendar->GetWidth(),
        game_ui_tome_calendar->GetHeight(),
        1, 0, UIMSG_OpenCalendar, 0, pKeyActionMap->GetActionVKey(INPUT_TimeCal),
      localization->GetString(78), { {game_ui_tome_calendar} });//Calendar

    game_ui_tome_storyline = assets->GetImage_16BitColorKey("ib-td5-A", 0x7FF);
    pBtn_History = pPrimaryWindow->CreateButton(
        600, 361,
        game_ui_tome_storyline->GetWidth(),
        game_ui_tome_storyline->GetHeight(),
        1, 0, UIMSG_OpenHistoryBook, 0, 'H',
      localization->GetString(602), { {game_ui_tome_storyline} });//History

    bFlashAutonotesBook = 0;
    bFlashQuestBook = 0;
    bFlashHistoryBook = 0;

    pBtn_ZoomIn = pPrimaryWindow->CreateButton(574, 136, game_ui_btn_zoomin->GetWidth(),
        game_ui_btn_zoomin->GetHeight(), 2, 0, UIMSG_ClickZoomInBtn, 0, pKeyActionMap->GetActionVKey(INPUT_ZoomIn),
      localization->GetString(252), { {game_ui_btn_zoomin} }); // Zoom In

    pBtn_ZoomOut = pPrimaryWindow->CreateButton(519, 136, game_ui_btn_zoomout->GetWidth(),
        game_ui_btn_zoomout->GetHeight(), 2, 0, UIMSG_ClickZoomOutBtn, 0, pKeyActionMap->GetActionVKey(INPUT_ZoomOut),
      localization->GetString(251), { {game_ui_btn_zoomout} }); // Zoom Out

    pPrimaryWindow->CreateButton(481, 0, 153, 67, 1, 92, UIMSG_0, 0, 0, "");
    pPrimaryWindow->CreateButton(491, 149, 64, 74, 1, 0, UIMSG_StartHireling1Dialogue, 0, '5', "");
    pPrimaryWindow->CreateButton(561, 149, 64, 74, 1, 0, UIMSG_StartHireling2Dialogue, 0, '6', "");
    pPrimaryWindow->CreateButton(476, 322, 77, 17, 1, 100, UIMSG_0, 0, 0, "");
    pPrimaryWindow->CreateButton(555, 322, 77, 17, 1, 101, UIMSG_0, 0, 0, "");

    pBtn_CastSpell = pPrimaryWindow->CreateButton(476, 450,
        game_ui_btn_cast->GetWidth(),
        game_ui_btn_cast->GetHeight(),
      1, 0, UIMSG_SpellBookWindow, 0, 67, localization->GetString(38), { {game_ui_btn_cast} });
    pBtn_Rest = pPrimaryWindow->CreateButton(518, 450,
        game_ui_btn_rest->GetWidth(),
        game_ui_btn_rest->GetHeight(),
      1, 0, UIMSG_RestWindow, 0, 82, localization->GetString(182), { {game_ui_btn_rest} });
    pBtn_QuickReference = pPrimaryWindow->CreateButton(560, 450,
        game_ui_btn_quickref->GetWidth(),
        game_ui_btn_quickref->GetHeight(),
      1, 0, UIMSG_QuickReference, 0, 90, localization->GetString(173), { {game_ui_btn_quickref} });
    pBtn_GameSettings = pPrimaryWindow->CreateButton(602, 450,
        game_ui_btn_settings->GetWidth(),
        game_ui_btn_settings->GetHeight(),
      1, 0, UIMSG_GameMenuButton, 0, 0, localization->GetString(93), { {game_ui_btn_settings} });

    pBtn_NPCLeft = pPrimaryWindow->CreateButton(469, 178,
        ui_btn_npc_left->GetWidth(),
        ui_btn_npc_left->GetHeight(),
      1, 0, UIMSG_ScrollNPCPanel, 0, 0, "", { {ui_btn_npc_left} });
    pBtn_NPCRight = pPrimaryWindow->CreateButton(626, 178,
        ui_btn_npc_right->GetWidth(),
        ui_btn_npc_right->GetHeight(),
      1, 0, UIMSG_ScrollNPCPanel, 1, 0, "", { {ui_btn_npc_right} });

    LoadPartyBuffIcons();
}
