#include "GUI/UI/UIMainMenu.h"

#include "Engine/EngineGlobals.h"
#include "Engine/Localization.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIMessageQueue.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/MediaPlayer.h"

GUIWindow_MainMenu::GUIWindow_MainMenu() :
    GUIWindow(WINDOW_MainMenu, {0, 0}, render->GetRenderDimensions()) {
    main_menu_background = assets->getImage_PCXFromIconsLOD("title.pcx");

    ui_mainmenu_new = assets->getImage_ColorKey("title_new");
    ui_mainmenu_load = assets->getImage_ColorKey("title_load");
    ui_mainmenu_credits = assets->getImage_ColorKey("title_cred");
    ui_mainmenu_exit = assets->getImage_ColorKey("title_exit");

    pBtnNew = CreateButton("MainMenu_NewGame", {495, 172}, ui_mainmenu_new->size(), 1, 0,
                           UIMSG_MainMenu_ShowPartyCreationWnd, 0, Io::InputAction::NewGame, "", {ui_mainmenu_new});
    pBtnLoad = CreateButton("MainMenu_LoadGame", {495, 227}, ui_mainmenu_load->size(), 1, 0,
                            UIMSG_MainMenu_ShowLoadWindow, 1, Io::InputAction::LoadGame, "", {ui_mainmenu_load});
    pBtnCredits = CreateButton("MainMenu_Credits", {495, 282}, ui_mainmenu_credits->size(), 1, 0,
                               UIMSG_ShowCredits, 2, Io::InputAction::Credits, "", {ui_mainmenu_credits});
    pBtnExit = CreateButton("MainMenu_ExitGame", {495, 337}, ui_mainmenu_exit->size(), 1, 0,
                            UIMSG_ExitToWindows, 3, Io::InputAction::ExitGame, "", {ui_mainmenu_exit});
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

    Pointi pt = mouse->GetCursorPos();

    GraphicsImage *pTexture = nullptr;
    if (!pGameOverWindow) {  // Really why???
        for (GUIButton *pButton : vButtons) {
            if (pButton->Contains(pt.x, pt.y)) {
                auto pControlParam = pButton->msg_param;
                int pY = 0;
                switch (pControlParam) {  // backlight for buttons
                    case 0:
                        pTexture = assets->getImage_ColorKey("title_new");
                        pY = 172;
                        break;
                    case 1:
                        pTexture = assets->getImage_ColorKey("title_load");
                        pY = 227;
                        break;
                    case 2:
                        pTexture = assets->getImage_ColorKey("title_cred");
                        pY = 282;
                        break;
                    case 3:
                        pTexture = assets->getImage_ColorKey("title_exit");
                        pY = 337;
                        break;
                }
                render->DrawTextureNew(495 / float(render->GetRenderDimensions().w), pY / float(render->GetRenderDimensions().h), pTexture);
            }
        }
    }
}

void GUIWindow_MainMenu::processMessage(UIMessageType message) {
    // Play the sound and change visual connected to the related button
    switch (message) {
    case UIMSG_MainMenu_ShowPartyCreationWnd:
        new OnButtonClick2({ 495, 172 }, { 0, 0 }, pBtnNew);
        break;
    case UIMSG_MainMenu_ShowLoadWindow:
        new OnButtonClick2({ 495, 227 }, { 0, 0 }, pBtnLoad);
        break;
    case UIMSG_ShowCredits:
        new OnButtonClick2({ 495, 282 }, { 0, 0 }, pBtnCredits);
        break;
    case UIMSG_ExitToWindows:
        new OnButtonClick2({ 495, 337 }, { 0, 0 }, pBtnExit);
        break;
    default:
        break;
    }
}

void GUIWindow_MainMenu::EventLoop() {
    while (engine->_messageQueue->haveMessages()) {
        UIMessageType pUIMessageType;
        int pParam;
        int param2;
        engine->_messageQueue->popMessage(&pUIMessageType, &pParam, &param2);

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

void GUIWindow_MainMenu::drawMM7CopyrightWindow() {
    GUIWindow Dst;
    Dst.uFrameWidth = 624;
    Dst.uFrameHeight = 256;
    Dst.uFrameX = 8;
    Dst.uFrameY = 30;
    Dst.uFrameHeight = assets->pFontSmallnum->CalcTextHeight(localization->GetString(LSTR_3DO_COPYRIGHT), Dst.uFrameWidth, 24, 0);
    Dst.uFrameHeight += 2 * assets->pFontSmallnum->GetHeight() + 24;
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
    Dst.DrawTitleText(assets->pFontSmallnum.get(), 0, 12, ui_mainmenu_copyright_color, localization->GetString(LSTR_3DO_COPYRIGHT), 3);
}

void GUIWindow_MainMenu::drawCopyrightAndInit(std::function<void()> initFunc) {
    GraphicsImage *tex = assets->getImage_PCXFromIconsLOD("mm6title.pcx");

    render->ResetUIClipRect();
    render->BeginScene2D();
    {
        render->DrawTextureNew(0, 0, tex);
        drawMM7CopyrightWindow();
        render->Present();

        initFunc();

        tex->Release();
    }
}
