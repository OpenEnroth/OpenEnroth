#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/SaveLoad.h"
#include "Engine/Time.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"

#include "IO/Keyboard.h"
#include "IO/Mouse.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIMainMenu.h"
#include "GUI/UI/UIPartyCreation.h"
#include "GUI/UI/UISaveLoad.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/MediaPlayer.h"

#include "Game/MainMenu.h"

Image *main_menu_bg = nullptr;
Image *main_menu_background = nullptr;

void MainMenu_EventLoop() {
    Player *pPlayer;               // ebx@2
    int v4;                        // eax@29
    int v15;                       // edi@70
    char v20;                      // dl@116
    unsigned int v25;              // eax@120
    int pParam;                    // [sp+4h] [bp-Ch]@3
    UIMessageType pUIMessageType;  // [sp+8h] [bp-8h]@3

    if (!pMessageQueue_50CBD0->Empty()) {
        pPlayer = pParty->pPlayers.data();
        do {
            int param2;
            pMessageQueue_50CBD0->PopMessage(&pUIMessageType, &pParam, &param2);
            // auto player = &pParty->pPlayers[pParam];

            switch (pUIMessageType) {  // For buttons of window MainMenu
                case UIMSG_MainMenu_ShowPartyCreationWnd:
                    new OnButtonClick2(495, 172, 0, 0, (int)pMainMenu_BtnNew);
                    SetCurrentMenuID(MENU_NEWGAME);
                    break;
                case UIMSG_MainMenu_ShowLoadWindow:
                    new OnButtonClick2(495, 227, 0, 0, (int)pMainMenu_BtnLoad);
                    SetCurrentMenuID(MENU_SAVELOAD);
                    break;
                case UIMSG_ShowCredits:
                    new OnButtonClick2(495, 282, 0, 0,
                                       (int)pMainMenu_BtnCredits);
                    SetCurrentMenuID(MENU_CREDITS);
                    break;
                case UIMSG_ExitToWindows:
                    new OnButtonClick2(495, 337, 0, 0, (int)pMainMenu_BtnExit);
                    SetCurrentMenuID(MENU_EXIT_GAME);
                    break;
                case UIMSG_ChangeGameState:
                    uGameState = GAME_FINISHED;
                    break;
                case UIMSG_ChangeCursor:
                    pMouse->SetCursorImage("MICON2");
                    break;
                case UIMSG_DebugBlv:
                    __debugbreak();  // some debugging tool
                    SetCurrentMenuID(MENU_DebugBLVLevel);
                    break;
                case UIMSG_Cancel:
                    new OnCancel3(pGUIWindow_CurrentMenu->uFrameX + 350,
                                  pGUIWindow_CurrentMenu->uFrameY + 302, 61, 28,
                                  (int)pBtnCancel);
                    break;
                case UIMSG_AD:
                    __debugbreak();  // some debugging tool
                    new OnButtonClick2(pMainMenu_BtnNew->uX,
                                       pMainMenu_BtnNew->uY, 0, 0,
                                       (int)pMainMenu_BtnNew);
                    SetCurrentMenuID(MENU_LoadingProcInMainMenu);
                    break;
                case UIMSG_DebugBlv2:
                    __debugbreak();  // some debugging tool
                    new OnButtonClick2(pMainMenu_BtnExit->uX,
                                       pMainMenu_BtnExit->uY, 0, 0,
                                       (int)pMainMenu_BtnExit);
                    SetCurrentMenuID(MENU_DebugBLVLevel);
                    break;
                case UIMSG_Escape:
                    if (pModalWindow) {
                        pModalWindow->Release();
                        pModalWindow = nullptr;
                        break;
                    }
                    if (!(dword_6BE364_game_settings_1 & GAME_SETTINGS_4000))
                        break;
                    v15 = 1;
                    pMediaPlayer->StopMovie();
                    viewparams->bRedrawGameUI = 1;
                    viewparams->field_48 = 1;
                    if (GetCurrentMenuID() == MENU_MAIN ||
                        GetCurrentMenuID() == MENU_MMT_MAIN_MENU ||
                        GetCurrentMenuID() == MENU_CREATEPARTY ||
                        GetCurrentMenuID() == MENU_NAMEPANELESC) {
                        // if ( current_screen_type == SCREEN_VIDEO )
                        // pVideoPlayer->FastForwardToFrame(pVideoPlayer->pResetflag);
                        if (GetCurrentMenuID() ==
                            MENU_NAMEPANELESC) {  // из панели изменения имени
                            SetCurrentMenuID(
                                MENU_CREATEPARTY);  //в окно создания группы
                            break;
                        }
                        if (GetCurrentMenuID() ==
                            MENU_CREDITSPROC) {  // из окна Создатели
                            SetCurrentMenuID(
                                MENU_CREDITSCLOSE);  //в закрытие Создатели
                            break;
                        }
                        pMessageQueue_50CBD0->AddGUIMessage(
                            UIMSG_ChangeGameState, 0, 0);
                        break;
                    }
                    if (GetCurrentMenuID() == MENU_CREDITSPROC &&
                        current_screen_type == SCREEN_GAME) {
                        // if ( current_screen_type == SCREEN_VIDEO )
                        // pVideoPlayer->FastForwardToFrame(pVideoPlayer->pResetflag);
                        if (GetCurrentMenuID() == MENU_NAMEPANELESC) {
                            SetCurrentMenuID(MENU_CREATEPARTY);
                            break;
                        }
                        if (GetCurrentMenuID() == MENU_CREDITSPROC) {
                            SetCurrentMenuID(MENU_CREDITSCLOSE);
                            break;
                        }
                        pMessageQueue_50CBD0->AddGUIMessage(
                            UIMSG_ChangeGameState, 0, 0);
                        break;
                    }
                    /*if (current_screen_type == SCREEN_LOADGAME)
                    {
                    //crt_deconstruct_ptr_6A0118();
                    main_menu_background.Release();
                    main_menu_background.Load("title.pcx", 0);
                    SetCurrentMenuID(MENU_MAIN);
                    v15 = 1;
                    pGUIWindow_CurrentMenu->Release();
                    pGUIWindow2->Release();
                    pGUIWindow2 = 0;
                    pEventTimer->Resume();
                    current_screen_type = SCREEN_GAME;
                    viewparams->bRedrawGameUI = v15;
                    break;
                    }*/
                    if (current_screen_type == SCREEN_VIDEO) {
                        // pVideoPlayer->Unload();
                    } else {
                        if (current_screen_type !=
                            SCREEN_1B) {  // MENU_CREDITSCLOSE  for example
                            if (pGUIWindow_CurrentMenu) {
                                pGUIWindow_CurrentMenu->Release();
                                pGUIWindow_CurrentMenu = nullptr;
                            }
                            if (pGUIWindow2) {
                                pGUIWindow2->Release();
                                pGUIWindow2 = 0;
                            }
                            pEventTimer->Resume();
                            current_screen_type = SCREEN_GAME;
                            viewparams->bRedrawGameUI = v15;
                            break;
                        }
                        // VideoPlayer::dtor();
                    }
                    break;
                default:
                    break;
            }
        } while (!pMessageQueue_50CBD0->Empty());
    }
}

static bool first_initialization = true;

void MainMenu_Loop() {
    GUIButton *pButton;          // eax@27
    unsigned int pControlParam;  // ecx@35
    unsigned int pY;             // [sp-18h] [bp-54h]@39
    Texture_MM7 *pTexture;       // [sp-14h] [bp-50h]@39
    GUIWindow *pWindow;          // [sp+4h] [bp-38h]@11

    pAudioPlayer->StopChannels(-1, -1);
    if (!bNoSound && pAudioPlayer) pAudioPlayer->MusicPlayTrack(MUSIC_MainMenu);

    if (first_initialization) {
        first_initialization = false;

        if (!main_menu_bg) {
            main_menu_bg = assets->GetImage_PCXFromIconsLOD("mm6title.pcx");
        }

        render->ResetUIClipRect();
        render->BeginScene();
        { DrawMM7CopyrightWindow(); }
        render->EndScene();
        render->Present();

#ifdef NDEBUG
        Sleep(1500);  // let the copyright window stay for a while
#endif

        SecondaryInitialization();
        FinalInitialization();
    }

    current_screen_type = SCREEN_GAME;

    pGUIWindow2 = 0;

    pWindow_MainMenu = new GUIWindow_MainMenu();

    SetCurrentMenuID(MENU_MAIN);
    window->Activate();

    while (GetCurrentMenuID() == MENU_MAIN) {
        Point pt = pMouse->GetCursorPos();
        pWindow = pWindow_MainMenu;

        OS_PeekMessageLoop();
        if (dword_6BE364_game_settings_1 & GAME_SETTINGS_APP_INACTIVE) {
            OS_WaitMessage();
            continue;
        }

        if (!main_menu_background) {
            main_menu_background =
                assets->GetImage_PCXFromIconsLOD("title.pcx");
        }

        render->BeginScene();
        {
            render->DrawTextureNew(0, 0, main_menu_background);

            MainMenu_EventLoop();
            GUI_UpdateWindows();
        }
        render->EndScene();
        render->Present();
    }

    render->BeginScene();
    {
        MainMenu_EventLoop();
        GUI_UpdateWindows();
    }
    render->EndScene();
    render->Present();

    main_menu_background->Release();
    main_menu_background = nullptr;

    pWindow_MainMenu->Release();
    pWindow_MainMenu = nullptr;
}

void DrawMM7CopyrightWindow() {
    if (!main_menu_background) {
        main_menu_background = assets->GetImage_PCXFromIconsLOD("title.pcx");
    }

    render->DrawTextureNew(0, 0, main_menu_background);

    GUIWindow Dst;  // [sp+8h] [bp-54h]@1

    memset(&Dst, 0, sizeof(Dst));
    Dst.uFrameWidth = 624;
    Dst.uFrameHeight = 256;
    Dst.uFrameX = 8;
    Dst.uFrameY = 30;  // c 1999 The 3DO Company.
    Dst.uFrameHeight =
        pFontSmallnum->CalcTextHeight(localization->GetString(157),
                                      Dst.uFrameWidth, 24, 0) +
        2 * (unsigned char)pFontSmallnum->GetHeight() + 24;
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
    Dst.DrawTitleText(pFontSmallnum, 0, 12, ui_mainmenu_copyright_color,
                      localization->GetString(157), 3);
}
