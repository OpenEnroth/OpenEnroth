#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"
#include "Engine/Localization.h"
#include "Engine/Time.h"
#include "Engine/Party.h"
#include "Engine/LOD.h"
#include "Engine/SaveLoad.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"

#include "IO/Mouse.h"
#include "IO/Keyboard.h"

#include "GUI/UI/UIMainMenu.h"
#include "GUI/UI/UIPartyCreation.h"
#include "GUI/UI/UISaveLoad.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/Audio/AIL.h"

#include "Game/MainMenu.h"
#include "Game/MainMenuLoad.h"


Image *main_menu_bg = nullptr;
Image *main_menu_background = nullptr;



//----- (00435748) --------------------------------------------------------
void MainMenu_EventLoop()
{
    Player *pPlayer; // ebx@2
    void *v3; // edi@21
    signed int v4; // eax@29
    //  int v5; // ecx@29
    //  PLAYER_SKILL_TYPE v6; // edi@37
    GUIWindow *pWindow; // eax@56
    GUIButton *pButton; // eax@59
    int v15; // edi@70
    char v20; // dl@116
    unsigned int v21; // eax@116
    unsigned int v25; // eax@120
    unsigned int v26; // ecx@127
    //  SoundID pSoundID; // [sp-2Ch] [bp-3Ch]@36
    //  signed int v41; // [sp-10h] [bp-20h]@29
    int pParam; // [sp+4h] [bp-Ch]@3
    UIMessageType pUIMessageType; // [sp+8h] [bp-8h]@3
    int pSex; // [sp+Ch] [bp-4h]@3

    if (pMessageQueue_50CBD0->uNumMessages)
    {
        pPlayer = pParty->pPlayers.data();
        do
        {
            int param2;
            pMessageQueue_50CBD0->PopMessage(&pUIMessageType, &pParam, &param2);
            //auto player = &pParty->pPlayers[pParam];

            switch (pUIMessageType) // For buttons of window MainMenu
            {
            case UIMSG_MainMenu_ShowPartyCreationWnd:
                new OnButtonClick2(495, 172, 0, 0, (int)pMainMenu_BtnNew, 0);
                SetCurrentMenuID(MENU_NEWGAME);
                break;
            case UIMSG_MainMenu_ShowLoadWindow:
                new OnButtonClick2(495, 227, 0, 0, (int)pMainMenu_BtnLoad, 0);
                SetCurrentMenuID(MENU_SAVELOAD);
                break;
            case UIMSG_ShowCredits:
                new OnButtonClick2(495, 282, 0, 0, (int)pMainMenu_BtnCredits, 0);
                SetCurrentMenuID(MENU_CREDITS);
                break;
            case UIMSG_ExitToWindows:
                new OnButtonClick2(495, 337, 0, 0, (int)pMainMenu_BtnExit, 0);
                SetCurrentMenuID(MENU_EXIT_GAME);
                break;
            /*case UIMSG_PlayerCreation_SelectAttribute:
                pGUIWindow_CurrentMenu->pCurrentPosActiveItem = (pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem)
                    % 7 + pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * pParam;
                uPlayerCreationUI_SelectedCharacter = pParam;
                pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0, 0, -1, 0, 0, 0, 0);
                break;
            case UIMSG_PlayerCreation_VoicePrev:
                pSex = pParty->pPlayers[pParam].GetSexByVoice();
                do
                {
                    if (pParty->pPlayers[pParam].uVoiceID == 0)
                        pParty->pPlayers[pParam].uVoiceID = 19;
                    else --pParty->pPlayers[pParam].uVoiceID;
                } while (pParty->pPlayers[pParam].GetSexByVoice() != pSex);
                pButton = pCreationUI_BtnPressLeft2[pParam];
                new OnButtonClick(pButton->uX, pButton->uY, 0, 0, (int)pButton, (char *)1);
                pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0, 0, -1, 0, 0, 0, 0);
                pParty->pPlayers[pParam].PlaySound(SPEECH_PickMe, 0);
                break;
            case UIMSG_PlayerCreation_VoiceNext:
                pSex = pParty->pPlayers[pParam].GetSexByVoice();
                do
                {
                    pParty->pPlayers[pParam].uVoiceID = (pParty->pPlayers[pParam].uVoiceID + 1) % 20;
                } while (pParty->pPlayers[pParam].GetSexByVoice() != pSex);
                pButton = pCreationUI_BtnPressRight2[pParam];
                new OnButtonClick(pButton->uX, pButton->uY, 0, 0, (int)pButton, (char *)1);
                pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0, 0, -1, 0, 0, 0, 0);
                pParty->pPlayers[pParam].PlaySound(SPEECH_PickMe, 0);
                break;
            case UIMSG_PlayerCreation_FacePrev:
                //pPlayer = &pParty->pPlayers[pParam];
                if (!pParty->pPlayers[pParam].uCurrentFace)
                    pParty->pPlayers[pParam].uCurrentFace = 19;
                else
                    pParty->pPlayers[pParam].uCurrentFace -= 1;
                pParty->pPlayers[pParam].uVoiceID = pParty->pPlayers[pParam].uCurrentFace;
                pParty->pPlayers[pParam].SetInitialStats();
                pParty->pPlayers[pParam].SetSexByVoice();
                pParty->pPlayers[pParam].RandomizeName();
                v25 = pParam;
                pGUIWindow_CurrentMenu->pCurrentPosActiveItem = (pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem)
                    % 7 + pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * pParam;
                uPlayerCreationUI_SelectedCharacter = v25;
                new OnButtonClick(pCreationUI_BtnPressLeft[v25]->uX, pCreationUI_BtnPressLeft[v25]->uY, 0, 0, (int)pCreationUI_BtnPressLeft[v25], (char *)1);
                pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0, 0, -1, 0, 0, 0.0, 0);
                pParty->pPlayers[pParam].PlaySound(SPEECH_PickMe, 0);
                break;
            case UIMSG_PlayerCreation_FaceNext:
                //pPlayer = &pParty->pPlayers[pParam];
                v20 = (char)((int)pParty->pPlayers[pParam].uCurrentFace + 1) % 20;
                pParty->pPlayers[pParam].uCurrentFace = v20;
                pParty->pPlayers[pParam].uVoiceID = v20;
                pParty->pPlayers[pParam].SetInitialStats();
                pParty->pPlayers[pParam].SetSexByVoice();
                pParty->pPlayers[pParam].RandomizeName();
                v21 = pParam;
                pGUIWindow_CurrentMenu->pCurrentPosActiveItem = (pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem)
                    % 7 + pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * pParam;
                uPlayerCreationUI_SelectedCharacter = v21;
                new OnButtonClick(pCreationUI_BtnPressRight[v21]->uX, pCreationUI_BtnPressRight[v21]->uY, 0, 0, (int)pCreationUI_BtnPressRight[v21], (char *)1);
                pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0, 0, -1, 0, 0, 0, 0);
                pParty->pPlayers[pParam].PlaySound(SPEECH_PickMe, 0);
                break;
            case UIMSG_PlayerCreationClickPlus:
                new OnButtonClick2(613, 393, 0, 0, (int)pPlayerCreationUI_BtnPlus, (char *)1);
                pPlayer[uPlayerCreationUI_SelectedCharacter].IncreaseAttribute((pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem) % 7);
                pAudioPlayer->PlaySound(SOUND_ClickMinus, 0, 0, -1, 0, 0, 0, 0);
                break;
            case UIMSG_PlayerCreationClickMinus:
                new OnButtonClick2(523, 393, 0, 0, (int)pPlayerCreationUI_BtnMinus, (char *)1);
                pPlayer[uPlayerCreationUI_SelectedCharacter].DecreaseAttribute((pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem) % 7);
                pAudioPlayer->PlaySound(SOUND_ClickPlus, 0, 0, -1, 0, 0, 0, 0);
                break;
            case UIMSG_PlayerCreationSelectActiveSkill:
                if (pPlayer[uPlayerCreationUI_SelectedCharacter].GetSkillIdxByOrder(3) == 37)
                    pParty->pPlayers[uPlayerCreationUI_SelectedCharacter].pActiveSkills[pPlayer[uPlayerCreationUI_SelectedCharacter].GetSkillIdxByOrder(pParam + 4)] = 1;
                pAudioPlayer->PlaySound(SOUND_ClickSkill, 0, 0, -1, 0, 0, 0, 0);
                break;
            case UIMSG_PlayerCreationSelectClass:
                pPlayer[uPlayerCreationUI_SelectedCharacter].Reset((PLAYER_CLASS_TYPE)pParam);
                pAudioPlayer->PlaySound(SOUND_SelectingANewCharacter, 0, 0, -1, 0, 0, 0, 0);
                break;
            case UIMSG_PlayerCreationClickOK:
                new OnButtonClick2(580, 431, 0, 0, (int)pPlayerCreationUI_BtnOK, 0);
                if (PlayerCreation_GetUnspentAttributePointCount() || !PlayerCreation_Choose4Skills())
                    game_ui_status_bar_event_string_time_left = GetTickCount() + 4000;
                else
                    uGameState = GAME_STATE_STARTING_NEW_GAME;
                break;
            case UIMSG_PlayerCreationClickReset:
                new OnButtonClick2(527, 431, 0, 0, (int)pPlayerCreationUI_BtnReset, 0);
                pParty->Reset();
                break;
            case UIMSG_PlayerCreationRemoveUpSkill:
                v4 = pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem;
                pGUIWindow_CurrentMenu->pCurrentPosActiveItem = v4 % 7 + pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * pParam;
                if (pPlayer[pParam].GetSkillIdxByOrder(2) != 37)//37 - None(Нет)
                    pParty->pPlayers[pParam].pActiveSkills[pPlayer[pParam].GetSkillIdxByOrder(2)] = 0;
                break;
            case UIMSG_PlayerCreationRemoveDownSkill:
                v4 = pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem;
                pGUIWindow_CurrentMenu->pCurrentPosActiveItem = v4 % 7 + pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * pParam;
                if (pPlayer[pParam].GetSkillIdxByOrder(3) != 37)//37 - None(Нет)
                    pParty->pPlayers[pParam].pActiveSkills[pPlayer[pParam].GetSkillIdxByOrder(3)] = 0;
                break;
            case UIMSG_PlayerCreationChangeName:
                pAudioPlayer->PlaySound(SOUND_ClickSkill, 0, 0, -1, 0, 0, 0, 0);
                v3 = (void *)pParam;
                uPlayerCreationUI_SelectedCharacter = pParam;
                pKeyActionMap->EnterText(0, 15, pGUIWindow_CurrentMenu);
                pGUIWindow_CurrentMenu->ptr_1C = v3;
                break;*/
            case UIMSG_ChangeGameState:
                uGameState = GAME_FINISHED;
                break;
            case UIMSG_ChangeCursor:
                pMouse->SetCursorImage("MICON2");
                break;
            case UIMSG_DebugBlv:
                __debugbreak();//some debugging tool
                SetCurrentMenuID(MENU_DebugBLVLevel);
                break;
            /*case UIMSG_LoadGame:
                if (!pSavegameUsedSlots[uLoadGameUI_SelectedSlot])
                    break;
                SetCurrentMenuID(MENU_LoadingProcInMainMenu);
                break;
            case UIMSG_SelectLoadSlot:
                //main menu save/load wnd   clicking on savegame lines
                if (pGUIWindow_CurrentMenu->receives_keyboard_input_2 == WINDOW_INPUT_IN_PROGRESS)
                    pKeyActionMap->SetWindowInputStatus(WINDOW_INPUT_NONE);
                if (current_screen_type != SCREEN_SAVEGAME || uLoadGameUI_SelectedSlot != pParam + pSaveListPosition)
                {
                    //load clicked line
                    v26 = pParam + pSaveListPosition;
                    if (dword_6BE138 == pParam + pSaveListPosition)
                    {
                        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_SaveLoadBtn, 0, 0);
                        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_LoadGame, 0, 0);
                    }
                    uLoadGameUI_SelectedSlot = v26;
                    dword_6BE138 = v26;
                }
                else
                {
                    //typing in the line
                    pKeyActionMap->EnterText(0, 19, pGUIWindow_CurrentMenu);
                    strcpy(pKeyActionMap->pPressedKeysBuffer, pSavegameHeader[uLoadGameUI_SelectedSlot].pName);
                    pKeyActionMap->uNumKeysPressed = strlen(pKeyActionMap->pPressedKeysBuffer);
                }
                break;
            case UIMSG_SaveLoadBtn:
                new OnSaveLoad(pGUIWindow_CurrentMenu->uFrameX + 241, pGUIWindow_CurrentMenu->uFrameY + 302, 61, 28, (int)pBtnLoadSlot, 0);
                break;
            case UIMSG_DownArrow:
                ++pSaveListPosition;
                if (pSaveListPosition >= pParam)
                    pSaveListPosition = pParam - 1;
                if (pSaveListPosition < 1)
                    pSaveListPosition = 0;
                pWindow = pGUIWindow_CurrentMenu;
                new OnButtonClick2(pWindow->uFrameX + 215, pGUIWindow_CurrentMenu->uFrameY + 323, 0, 0, (int)pBtnDownArrow, 0);
                break;*/
            case UIMSG_Cancel:
                new OnCancel3(pGUIWindow_CurrentMenu->uFrameX + 350, pGUIWindow_CurrentMenu->uFrameY + 302, 61, 28, (int)pBtnCancel, 0);
                break;
            /*case UIMSG_ArrowUp:
                --pSaveListPosition;
                if (pSaveListPosition < 0)
                    pSaveListPosition = 0;
                pWindow = pGUIWindow_CurrentMenu;
                new OnButtonClick2(pWindow->uFrameX + 215, pGUIWindow_CurrentMenu->uFrameY + 197, 0, 0, (int)pBtnArrowUp, 0);
                break;*/
            case UIMSG_AD:
                __debugbreak();//some debugging tool
                new OnButtonClick2(pMainMenu_BtnNew->uX, pMainMenu_BtnNew->uY, 0, 0, (int)pMainMenu_BtnNew, 0);
                SetCurrentMenuID(MENU_LoadingProcInMainMenu);
                break;
            case UIMSG_DebugBlv2:
                __debugbreak();//some debugging tool
                new OnButtonClick2(pMainMenu_BtnExit->uX, pMainMenu_BtnExit->uY, 0, 0, (int)pMainMenu_BtnExit, 0);
                SetCurrentMenuID(MENU_DebugBLVLevel);
                break;
            case UIMSG_Escape:
                if (pModalWindow)
                {
                    pModalWindow->Release();
                    pModalWindow = nullptr;
                    break;
                }
                if (!(dword_6BE364_game_settings_1 & GAME_SETTINGS_4000))
                    break;
                v15 = 1;
                pMediaPlayer->bStopBeforeSchedule = 1;
                viewparams->bRedrawGameUI = 1;
                viewparams->field_48 = 1;
                if (GetCurrentMenuID() == MENU_MAIN || GetCurrentMenuID() == MENU_MMT_MAIN_MENU
                    || GetCurrentMenuID() == MENU_CREATEPARTY || GetCurrentMenuID() == MENU_NAMEPANELESC)
                {
                    //if ( current_screen_type == SCREEN_VIDEO )
                    //pVideoPlayer->FastForwardToFrame(pVideoPlayer->pResetflag);			  
                    if (GetCurrentMenuID() == MENU_NAMEPANELESC)  //из панели изменения имени
                    {
                        SetCurrentMenuID(MENU_CREATEPARTY);//в окно создания группы
                        break;
                    }
                    if (GetCurrentMenuID() == MENU_CREDITSPROC)	//из окна Создатели
                    {
                        SetCurrentMenuID(MENU_CREDITSCLOSE);//в закрытие Создатели
                        break;
                    }
                    pMessageQueue_50CBD0->AddGUIMessage(UIMSG_ChangeGameState, 0, 0);
                    break;
                }
                if (GetCurrentMenuID() == MENU_CREDITSPROC && current_screen_type == SCREEN_GAME)
                {
                    //if ( current_screen_type == SCREEN_VIDEO )
                    //pVideoPlayer->FastForwardToFrame(pVideoPlayer->pResetflag);
                    if (GetCurrentMenuID() == MENU_NAMEPANELESC)
                    {
                        SetCurrentMenuID(MENU_CREATEPARTY);
                        break;
                    }
                    if (GetCurrentMenuID() == MENU_CREDITSPROC)
                    {
                        SetCurrentMenuID(MENU_CREDITSCLOSE);
                        break;
                    }
                    pMessageQueue_50CBD0->AddGUIMessage(UIMSG_ChangeGameState, 0, 0);
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
                if (current_screen_type == SCREEN_VIDEO)
                {
                    //pVideoPlayer->Unload();
                }
                else
                {
                    if (current_screen_type != SCREEN_1B)
                    { // MENU_CREDITSCLOSE  for example
                        if (pGUIWindow_CurrentMenu)
                        {
                            pGUIWindow_CurrentMenu->Release();
                            pGUIWindow_CurrentMenu = nullptr;
                        }
                        if (pGUIWindow2)
                        {
                            pGUIWindow2->Release();
                            pGUIWindow2 = 0;
                        }
                        pEventTimer->Resume();
                        current_screen_type = SCREEN_GAME;
                        viewparams->bRedrawGameUI = v15;
                        break;
                    }
                    //VideoPlayer::dtor();
                }
                break;
            default:
                break;
            }
        } while (pMessageQueue_50CBD0->uNumMessages);
    }
}


static bool first_initialization = true;
//----- (004627B7) --------------------------------------------------------
void MainMenu_Loop()
{
    GUIButton *pButton; // eax@27
    unsigned int pControlParam; // ecx@35
    unsigned int pY; // [sp-18h] [bp-54h]@39
    Texture_MM7 *pTexture; // [sp-14h] [bp-50h]@39
    GUIWindow *pWindow; // [sp+4h] [bp-38h]@11
    MSG msg;


    pAudioPlayer->StopChannels(-1, -1);
    if (!bNoSound && pAudioPlayer)
        pAudioPlayer->PlayMusicTrack(MUSIC_MainMenu);

    if (first_initialization)
    {
        first_initialization = false;

        if (!main_menu_bg)
        {
            main_menu_bg = assets->GetImage_PCXFromIconsLOD("mm6title.pcx");
        }

        render->ResetUIClipRect();
        render->BeginScene();
        {
            DrawMM7CopyrightWindow();
        }
        render->EndScene();
        render->Present();

#ifdef NDEBUG
        Sleep(1500);   // let the copyright window stay for a while
#endif

        SecondaryInitialization();
        FinalInitialization();
    }


    current_screen_type = SCREEN_GAME;

    pGUIWindow2 = 0;

    pWindow_MainMenu = new GUIWindow_MainMenu();

    SetCurrentMenuID(MENU_MAIN);
    SetForegroundWindow(window->GetApiHandle());
    SendMessageW(window->GetApiHandle(), WM_ACTIVATEAPP, 1, 0);

    while (GetCurrentMenuID() == MENU_MAIN)
    {
        POINT pt;
        pMouse->GetCursorPos(&pt);
        pWindow = pWindow_MainMenu;

        while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                Engine_DeinitializeAndTerminate(0);
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        if (dword_6BE364_game_settings_1 & GAME_SETTINGS_APP_INACTIVE)
        {
            WaitMessage();
            continue;
        }

        if (!main_menu_background)
        {
            main_menu_background = assets->GetImage_PCXFromIconsLOD("title.pcx");
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



//----- (00415485) --------------------------------------------------------
void DrawMM7CopyrightWindow()
{
    render->DrawTextureNew(0, 0, main_menu_bg);

    GUIWindow Dst; // [sp+8h] [bp-54h]@1

    memset(&Dst, 0, sizeof(Dst));
    Dst.uFrameWidth = 624;
    Dst.uFrameHeight = 256;
    Dst.uFrameX = 8;
    Dst.uFrameY = 30;                             // c 1999 The 3DO Company.
    Dst.uFrameHeight = pFontSmallnum->CalcTextHeight(localization->GetString(157), &Dst, 24, 0)
        + 2 * LOBYTE(pFontSmallnum->uFontHeight)
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
