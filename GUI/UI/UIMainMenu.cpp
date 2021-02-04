#include "GUI/UI/UIMainMenu.h"

#include "Engine/Point.h"
#include "Engine/Localization.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Image.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/MediaPlayer.h"

#include "Platform/OSWindow.h"


GUIWindow_MainMenu *pWindow_MainMenu = nullptr;

GUIWindow_MainMenu::GUIWindow_MainMenu() :
    GUIWindow(WINDOW_MainMenu, 0, 0, window->GetWidth(), window->GetHeight(), 0) {
    main_menu_background = assets->GetImage_PCXFromIconsLOD("title.pcx");

    ui_mainmenu_new = assets->GetImage_ColorKey("title_new", 0x7FF);
    ui_mainmenu_load = assets->GetImage_ColorKey("title_load", 0x7FF);
    ui_mainmenu_credits = assets->GetImage_ColorKey("title_cred", 0x7FF);
    ui_mainmenu_exit = assets->GetImage_ColorKey("title_exit", 0x7FF);

    pBtnNew = CreateButton(495, 172, ui_mainmenu_new->GetWidth(), ui_mainmenu_new->GetHeight(),
        1, 0, UIMSG_MainMenu_ShowPartyCreationWnd, 0, GameKey::N, "", { { ui_mainmenu_new } });
    pBtnLoad = CreateButton(495, 227, ui_mainmenu_load->GetWidth(), ui_mainmenu_load->GetHeight(),
        1, 0, UIMSG_MainMenu_ShowLoadWindow, 1, GameKey::L, "", { { ui_mainmenu_load } });
    pBtnCredits = CreateButton(495, 282, ui_mainmenu_credits->GetWidth(), ui_mainmenu_credits->GetHeight(),
        1, 0, UIMSG_ShowCredits, 2, GameKey::C, "", { { ui_mainmenu_credits } });
    pBtnExit = CreateButton(495, 337, ui_mainmenu_exit->GetWidth(), ui_mainmenu_exit->GetHeight(),
        1, 0, UIMSG_ExitToWindows, 3, GameKey::None, "", { { ui_mainmenu_exit } });
}

GUIWindow_MainMenu::~GUIWindow_MainMenu() {
    ui_mainmenu_new->Release();
    ui_mainmenu_load->Release();
    ui_mainmenu_credits->Release();
    ui_mainmenu_exit->Release();
    main_menu_background->Release();
}

void GUIWindow_MainMenu::Update() {
    render->DrawTextureNew(0, 0, main_menu_background);

    Point pt = mouse->GetCursorPos();
    GUIWindow *pWindow = this;

    Image *pTexture = nullptr;
    if (!pModalWindow) {  // ???
        for (GUIButton *pButton : pWindow->vButtons) {
            if (pButton->Contains(pt.x, pt.y) && pWindow == pWindow_MainMenu) {
                auto pControlParam = pButton->msg_param;
                int pY = 0;
                switch (pControlParam) {  // backlight for buttons
                    case 0:
                        pTexture = assets->GetImage_ColorKey("title_new", 0x7FF);
                        pY = 172;
                        break;
                    case 1:
                        pTexture = assets->GetImage_ColorKey("title_load", 0x7FF);
                        pY = 227;
                        break;
                    case 2:
                        pTexture = assets->GetImage_ColorKey("title_cred", 0x7FF);
                        pY = 282;
                        break;
                    case 3:
                        pTexture = assets->GetImage_ColorKey("title_exit", 0x7FF);
                        pY = 337;
                        break;
                }
                render->DrawTextureAlphaNew(495 / float(render->GetRenderWidth()), pY / float(render->GetRenderHeight()), pTexture);
            }
        }
    }
}

void GUIWindow_MainMenu::EventLoop() {
    while (!pMessageQueue_50CBD0->Empty()) {
        UIMessageType pUIMessageType;
        int pParam;
        int param2;
        pMessageQueue_50CBD0->PopMessage(&pUIMessageType, &pParam, &param2);

        switch (pUIMessageType) {  // For buttons of window MainMenu
        case UIMSG_MainMenu_ShowPartyCreationWnd:
            new OnButtonClick2(495, 172, 0, 0, pBtnNew);
            SetCurrentMenuID(MENU_NEWGAME);
            break;
        case UIMSG_MainMenu_ShowLoadWindow:
            new OnButtonClick2(495, 227, 0, 0, pBtnLoad);
            SetCurrentMenuID(MENU_SAVELOAD);
            break;
        case UIMSG_ShowCredits:
            new OnButtonClick2(495, 282, 0, 0, pBtnCredits);
            SetCurrentMenuID(MENU_CREDITS);
            break;
        case UIMSG_ExitToWindows:
            new OnButtonClick2(495, 337, 0, 0, pBtnExit);
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
    pAudioPlayer->StopChannels(-1, -1);
    pAudioPlayer->MusicPlayTrack(MUSIC_MainMenu);

    if (first_initialization) {
        first_initialization = false;

        Image *tex = assets->GetImage_PCXFromIconsLOD("mm6title.pcx");

        render->ResetUIClipRect();
        render->BeginScene();
        {
            render->DrawTextureNew(0, 0, tex);
            DrawMM7CopyrightWindow();
        }
        render->EndScene();
        render->Present();

        engine->SecondaryInitialization();
        FinalInitialization();

        tex->Release();
    }

    current_screen_type = CURRENT_SCREEN::SCREEN_GAME;

    pGUIWindow2 = nullptr;

    pWindow_MainMenu = new GUIWindow_MainMenu();

    SetCurrentMenuID(MENU_MAIN);
    // window->Activate();

    while (GetCurrentMenuID() == MENU_MAIN) {
        MessageLoopWithWait();

        render->BeginScene();
        {
            pWindow_MainMenu->EventLoop();
            GUI_UpdateWindows();
        }
        render->EndScene();
        render->Present();
    }

    pWindow_MainMenu->Release();
    delete pWindow_MainMenu;
    pWindow_MainMenu = nullptr;
}

void DrawMM7CopyrightWindow() {
    GUIWindow Dst;
    Dst.uFrameWidth = 624;
    Dst.uFrameHeight = 256;
    Dst.uFrameX = 8;
    Dst.uFrameY = 30;                             // c 1999 The 3DO Company.
    Dst.uFrameHeight = pFontSmallnum->CalcTextHeight(localization->GetString(157), Dst.uFrameWidth, 24, 0)
        + 2 * (unsigned char)pFontSmallnum->GetHeight()
        + 24;
    Dst.uFrameY = 470 - Dst.uFrameHeight;
    Dst.uFrameZ = Dst.uFrameX + Dst.uFrameWidth - 1;
    Dst.uFrameW = 469;
    Dst.DrawMessageBox(0);

    Dst.uFrameWidth -= 24;
    Dst.uFrameX += 12;
    Dst.uFrameY += 12;
    Dst.uFrameHeight -= 12;
    Dst.uFrameZ = Dst.uFrameX + Dst.uFrameWidth - 1;
    Dst.uFrameW = Dst.uFrameY + Dst.uFrameHeight - 1;
    Dst.DrawTitleText(pFontSmallnum, 0, 12, ui_mainmenu_copyright_color, localization->GetString(157), 3);
}
