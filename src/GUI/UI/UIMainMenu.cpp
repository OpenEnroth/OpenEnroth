#include "GUI/UI/UIMainMenu.h"

#include "Engine/EngineGlobals.h"
#include "Engine/Localization.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Image.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/MediaPlayer.h"



GUIWindow_MainMenu *pWindow_MainMenu = nullptr;

GUIWindow_MainMenu::GUIWindow_MainMenu() :
    GUIWindow(WINDOW_MainMenu, {0, 0}, render->GetRenderDimensions(), 0) {
    nuklear->Create(WINDOW_MainMenu);
    if (nuklear->Mode(WINDOW_MainMenu) == nuklear->NUKLEAR_MODE_EXCLUSIVE)
        return;

    main_menu_background = assets->GetImage_PCXFromIconsLOD("title.pcx");

    ui_mainmenu_new = assets->GetImage_ColorKey("title_new");
    ui_mainmenu_load = assets->GetImage_ColorKey("title_load");
    ui_mainmenu_credits = assets->GetImage_ColorKey("title_cred");
    ui_mainmenu_exit = assets->GetImage_ColorKey("title_exit");

    pBtnNew = CreateButton("MainMenu_NewGame", {495, 172}, {ui_mainmenu_new->GetWidth(), ui_mainmenu_new->GetHeight()}, 1, 0,
        UIMSG_MainMenu_ShowPartyCreationWnd, 0, InputAction::NewGame, "", {ui_mainmenu_new});
    pBtnLoad = CreateButton("MainMenu_LoadGame", {495, 227}, {ui_mainmenu_load->GetWidth(), ui_mainmenu_load->GetHeight()}, 1, 0,
        UIMSG_MainMenu_ShowLoadWindow, 1, InputAction::LoadGame, "", {ui_mainmenu_load});
    pBtnCredits = CreateButton("MainMenu_Credits", {495, 282}, {ui_mainmenu_credits->GetWidth(), ui_mainmenu_credits->GetHeight()}, 1, 0,
        UIMSG_ShowCredits, 2, InputAction::Credits, "", {ui_mainmenu_credits});
    pBtnExit = CreateButton("MainMenu_ExitGame", {495, 337}, {ui_mainmenu_exit->GetWidth(), ui_mainmenu_exit->GetHeight()}, 1, 0,
        UIMSG_ExitToWindows, 3, InputAction::ExitGame, "", {ui_mainmenu_exit});
}

GUIWindow_MainMenu::~GUIWindow_MainMenu() {
    if (nuklear->Mode(WINDOW_MainMenu) == nuklear->NUKLEAR_MODE_EXCLUSIVE) {
        nuklear->Release(WINDOW_MainMenu);
        return;
    }

    ui_mainmenu_new->Release();
    ui_mainmenu_load->Release();
    ui_mainmenu_credits->Release();
    ui_mainmenu_exit->Release();
    main_menu_background->Release();

    nuklear->Release(WINDOW_MainMenu);
}

void GUIWindow_MainMenu::Update() {
    nuklear->Draw(nuklear->NUKLEAR_STAGE_PRE, WINDOW_MainMenu, 2);
    if (nuklear->Mode(WINDOW_MainMenu) == nuklear->NUKLEAR_MODE_EXCLUSIVE) {
        nuklear->Draw(nuklear->NUKLEAR_STAGE_POST, WINDOW_MainMenu, 2);
        return;
    }

    render->DrawTextureNew(0, 0, main_menu_background);

    Pointi pt = mouse->GetCursorPos();
    GUIWindow *pWindow = this;

    Image *pTexture = nullptr;
    if (!pGameOverWindow) {  // ???
        for (GUIButton *pButton : pWindow->vButtons) {
            if (pButton->Contains(pt.x, pt.y) && pWindow == pWindow_MainMenu) {
                auto pControlParam = pButton->msg_param;
                int pY = 0;
                switch (pControlParam) {  // backlight for buttons
                    case 0:
                        pTexture = assets->GetImage_ColorKey("title_new");
                        pY = 172;
                        break;
                    case 1:
                        pTexture = assets->GetImage_ColorKey("title_load");
                        pY = 227;
                        break;
                    case 2:
                        pTexture = assets->GetImage_ColorKey("title_cred");
                        pY = 282;
                        break;
                    case 3:
                        pTexture = assets->GetImage_ColorKey("title_exit");
                        pY = 337;
                        break;
                }
                render->DrawTextureNew(495 / float(render->GetRenderDimensions().w), pY / float(render->GetRenderDimensions().h), pTexture);
            }
        }
    }

    nuklear->Draw(nuklear->NUKLEAR_STAGE_POST, WINDOW_MainMenu, 2);
}

void GUIWindow_MainMenu::EventLoop() {
    if (nuklear->Mode(WINDOW_MainMenu) == nuklear->NUKLEAR_MODE_EXCLUSIVE) {
        pCurrentFrameMessageQueue->Clear();
        return;
    }

    while (!pCurrentFrameMessageQueue->Empty()) {
        UIMessageType pUIMessageType;
        int pParam;
        int param2;
        pCurrentFrameMessageQueue->PopMessage(&pUIMessageType, &pParam, &param2);

        switch (pUIMessageType) {  // For buttons of window MainMenu
        case UIMSG_MainMenu_ShowPartyCreationWnd:
            new OnButtonClick2({495, 172}, {0, 0}, pBtnNew);
            SetCurrentMenuID(MENU_NEWGAME);
            break;
        case UIMSG_MainMenu_ShowLoadWindow:
            new OnButtonClick2({495, 227}, {0, 0}, pBtnLoad);
            SetCurrentMenuID(MENU_SAVELOAD);
            break;
        case UIMSG_ShowCredits:
            new OnButtonClick2({495, 282}, {0, 0}, pBtnCredits);
            SetCurrentMenuID(MENU_CREDITS);
            break;
        case UIMSG_ExitToWindows:
            new OnButtonClick2({495, 337}, {0, 0}, pBtnExit);
            SetCurrentMenuID(MENU_EXIT_GAME);
            break;
        case UIMSG_ChangeGameState:
            uGameState = GAME_FINISHED;
            break;
        default:
            break;
        }
    }
}

static bool first_initialization = true;

void GUIWindow_MainMenu::Loop() {
    Image *tex;
    nuklear->Create(WINDOW_MainMenu_Load);

    pAudioPlayer->PauseSounds(-1);
    pAudioPlayer->MusicPlayTrack(MUSIC_MainMenu);

    if (first_initialization) {
        first_initialization = false;

        if (nuklear->Mode(WINDOW_MainMenu_Load) != nuklear->NUKLEAR_MODE_EXCLUSIVE) {
            tex = assets->GetImage_PCXFromIconsLOD("mm6title.pcx");
        }

        render->ResetUIClipRect();
        render->BeginScene2D();
        {
            nuklear->Draw(nuklear->NUKLEAR_STAGE_PRE, WINDOW_MainMenu_Load, 1);
            if (nuklear->Mode(WINDOW_MainMenu_Load) != nuklear->NUKLEAR_MODE_EXCLUSIVE) {
                render->DrawTextureNew(0, 0, tex);
                DrawMM7CopyrightWindow();
            }
            nuklear->Draw(nuklear->NUKLEAR_STAGE_POST, WINDOW_MainMenu_Load, 1);
            render->Present();

            engine->SecondaryInitialization();
            FinalInitialization();

            if (nuklear->Mode(WINDOW_MainMenu_Load) != nuklear->NUKLEAR_MODE_EXCLUSIVE) {
                tex->Release();
            }
        }
    }
    nuklear->Release(WINDOW_MainMenu_Load);

    current_screen_type = CURRENT_SCREEN::SCREEN_GAME;

    pGUIWindow2 = nullptr;

    pWindow_MainMenu = new GUIWindow_MainMenu();

    SetCurrentMenuID(MENU_MAIN);
    // window->Activate();

    while (GetCurrentMenuID() == MENU_MAIN) {
        MessageLoopWithWait();

        render->BeginScene2D();
        {
            pWindow_MainMenu->EventLoop();
            GUI_UpdateWindows();
        }
        render->Present();
    }

    nuklear->Release(WINDOW_MainMenu);
    pWindow_MainMenu->Release();
    delete pWindow_MainMenu;
    pWindow_MainMenu = nullptr;
}

void DrawMM7CopyrightWindow() {
    GUIWindow Dst;
    Dst.uFrameWidth = 624;
    Dst.uFrameHeight = 256;
    Dst.uFrameX = 8;
    Dst.uFrameY = 30;
    Dst.uFrameHeight = pFontSmallnum->CalcTextHeight(
        localization->GetString(LSTR_3DO_COPYRIGHT), Dst.uFrameWidth, 24, 0)
        + 2 * (unsigned char)pFontSmallnum->GetHeight() + 24;
    Dst.uFrameY = 470 - Dst.uFrameHeight;
    Dst.uFrameZ = Dst.uFrameX + Dst.uFrameWidth - 1;
    Dst.uFrameW = 469;
    Dst.DrawMessageBox(0);

    Dst.uFrameWidth -= 28;
    Dst.uFrameX += 12;
    Dst.uFrameY += 12;
    Dst.uFrameHeight -= 12;
    Dst.uFrameZ = Dst.uFrameX + Dst.uFrameWidth - 1;
    Dst.uFrameW = Dst.uFrameY + Dst.uFrameHeight - 1;
    Dst.DrawTitleText(
        pFontSmallnum, 0, 12, ui_mainmenu_copyright_color,
        localization->GetString(LSTR_3DO_COPYRIGHT), 3);
}
