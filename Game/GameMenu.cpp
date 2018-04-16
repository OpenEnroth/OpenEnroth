#include "Game/GameMenu.h"

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
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIModal.h"
#include "GUI/UI/UISaveLoad.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

#include "Game/Game.h"

void Game_StartNewGameWhilePlaying(bool force_start) {
    if (dword_6BE138 == 124 || force_start) {
        pMessageQueue_50CBD0->Flush();
        // pGUIWindow_CurrentMenu->Release();
        uGameState = GAME_STATE_NEWGAME_OUT_GAMEMENU;
        current_screen_type = SCREEN_GAME;
        viewparams->bRedrawGameUI = 1;
    } else {
        GameUI_StatusBar_OnEvent(localization->GetString(
            201));  // "Are you sure?  Click again to start a New Game"
        pAudioPlayer->PlaySound(SOUND_quest, 0, 0, -1, 0, 0, 0, 0);
        dword_6BE138 = 124;
    }
}

void Game_QuitGameWhilePlaying(bool force_quit) {
    if (dword_6BE138 == 132 || force_quit) {
        pMessageQueue_50CBD0->Flush();
        // pGUIWindow_CurrentMenu->Release();
        current_screen_type = SCREEN_GAME;
        viewparams->bRedrawGameUI = 1;
        pAudioPlayer->PlaySound(SOUND_WoodDoorClosing, 0, 0, -1, 0, 0, 0, 0);
        uGameState = GAME_STATE_GAME_QUITTING_TO_MAIN_MENU;
    } else {
        GameUI_StatusBar_OnEvent(localization->GetString(
            82));  // "Are you sure?  Click again to quit"
        pAudioPlayer->PlaySound(SOUND_quest, 0, 0, -1, 0, 0, 0, 0);
        dword_6BE138 = 132;
    }
}

void Game_OpenLoadGameDialog() {
    pMessageQueue_50CBD0->Flush();
    pGUIWindow_CurrentMenu->Release();
    pGUIWindow_CurrentMenu = nullptr;

    // LoadUI_Load(1);
    current_screen_type = SCREEN_LOADGAME;
    pGUIWindow_CurrentMenu = new GUIWindow_Load(true);
}

void GameMenu_EventLoop() {
    while (!pMessageQueue_50CBD0->Empty()) {
        UIMessageType msg;
        int param, param2;
        pMessageQueue_50CBD0->PopMessage(&msg, &param, &param2);

        switch (msg) {
            case UIMSG_StartNewGame:
                Game_StartNewGameWhilePlaying(param);
                continue;
            case UIMSG_Quit:
                Game_QuitGameWhilePlaying(param);
                continue;
            case UIMSG_Game_OpenLoadGameDialog:
                Game_OpenLoadGameDialog();
                continue;

            case UIMSG_ArrowUp:
                --pSaveListPosition;
                if (pSaveListPosition < 0) pSaveListPosition = 0;
                new OnButtonClick2(215, 199, 17, 17, (int)pBtnArrowUp);
                continue;

            case UIMSG_DownArrow:
                ++pSaveListPosition;
                if (pSaveListPosition >= param) pSaveListPosition = param - 1;
                new OnButtonClick2(215, 323, 17, 17, (int)pBtnDownArrow);
                continue;

            case UIMSG_Cancel:
                new OnCancel(350, 302, 106, 42, (int)pBtnCancel);
                continue;

            case UIMSG_SaveLoadBtn:
                new OnSaveLoad(241, 302, 106, 42, (int)pBtnLoadSlot);
                continue;
            case UIMSG_SelectLoadSlot: {
                if (pGUIWindow_CurrentMenu->receives_keyboard_input_2 ==
                    WINDOW_INPUT_IN_PROGRESS)
                    pKeyActionMap->SetWindowInputStatus(WINDOW_INPUT_NONE);

                int v10 = pSaveListPosition + param;
                if (current_screen_type != SCREEN_SAVEGAME ||
                    uLoadGameUI_SelectedSlot != v10) {
                    if (dword_6BE138 == pSaveListPosition + param) {
                        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_SaveLoadBtn,
                                                            0, 0);
                        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_LoadGame, 0,
                                                            0);
                    }
                    uLoadGameUI_SelectedSlot = v10;
                    dword_6BE138 = v10;
                } else {
                    pKeyActionMap->EnterText(0, 19, pGUIWindow_CurrentMenu);
                    if (strcmp(pSavegameHeader[uLoadGameUI_SelectedSlot].pName,
                               localization->GetString(72)))  // "Empty"
                        strcpy(pKeyActionMap->pPressedKeysBuffer,
                               pSavegameHeader[uLoadGameUI_SelectedSlot].pName);
                    pKeyActionMap->uNumKeysPressed =
                        strlen(pKeyActionMap->pPressedKeysBuffer);
                }
            }
                continue;
            case UIMSG_LoadGame:
                if (pSavegameUsedSlots[uLoadGameUI_SelectedSlot]) {
                    LoadGame(uLoadGameUI_SelectedSlot);
                    uGameState = GAME_STATE_LOADING_GAME;
                }
                continue;
            case UIMSG_SaveGame:
                if (pGUIWindow_CurrentMenu->receives_keyboard_input_2 ==
                    WINDOW_INPUT_IN_PROGRESS) {
                    pKeyActionMap->SetWindowInputStatus(WINDOW_INPUT_NONE);
                    strcpy((char *)&pSavegameHeader[uLoadGameUI_SelectedSlot],
                           pKeyActionMap->pPressedKeysBuffer);
                }
                DoSavegame(uLoadGameUI_SelectedSlot);
                continue;
            case UIMSG_Game_OpenSaveGameDialog: {
                pGUIWindow_CurrentMenu->Release();

                current_screen_type = SCREEN_SAVEGAME;
                pGUIWindow_CurrentMenu =
                    new GUIWindow_Save();  // SaveUI_Load(current_screen_type =
                                           // SCREEN_SAVEGAME);
                continue;
            }
            case UIMSG_Game_OpenOptionsDialog:  // Open
            {
                pMessageQueue_50CBD0->Flush();

                pGUIWindow_CurrentMenu->Release();
                pGUIWindow_CurrentMenu =
                    new GUIWindow_GameOptions();  // GameMenuUI_Options_Load();

                viewparams->field_48 = 1;
                current_screen_type = SCREEN_OPTIONS;

                continue;
            }

            case UIMSG_OpenKeyMappingOptions:  // Open
            {
                pMessageQueue_50CBD0->Flush();

                pGUIWindow_CurrentMenu->Release();
                pGUIWindow_CurrentMenu =
                    new GUIWindow_GameKeyBindings();  // GameMenuUI_OptionsKeymapping_Load();

                viewparams->field_48 = 1;
                current_screen_type = SCREEN_KEYBOARD_OPTIONS;

                continue;
            }

            case UIMSG_ChangeKeyButton: {
                if (uGameMenuUI_CurentlySelectedKeyIdx != -1) {
                    pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0, 0, 0);
                } else {
                    uGameMenuUI_CurentlySelectedKeyIdx = param;
                    if (KeyboardPageNum != 1)
                        uGameMenuUI_CurentlySelectedKeyIdx += 14;
                    pKeyActionMap->EnterText(0, 1, pGUIWindow_CurrentMenu);
                }
                continue;
            }

            case UIMSG_ResetKeyMapping: {
                int v197 = 1;
                pKeyActionMap->SetDefaultMapping();
                for (uint i = 0; i < 28; i++) {
                    if (pKeyActionMap->GetActionVKey((enum InputAction)i) !=
                        pPrevVirtualCidesMapping[i]) {
                        if (v197) {
                            GUI_ReplaceHotkey(
                                (unsigned char)pPrevVirtualCidesMapping[i],
                                pKeyActionMap->GetActionVKey(
                                    (enum InputAction)i),
                                1);
                            v197 = 0;
                        } else {
                            GUI_ReplaceHotkey(
                                (unsigned char)pPrevVirtualCidesMapping[i],
                                pKeyActionMap->GetActionVKey(
                                (enum InputAction)i),
                                0);
                        }
                    }
                    pPrevVirtualCidesMapping[i] =
                        pKeyActionMap->GetActionVKey((enum InputAction)i);
                    GameMenuUI_InvaligKeyBindingsFlags[i] = false;
                }
                pAudioPlayer->PlaySound(SOUND_chimes, 0, 0, -1, 0, 0, 0, 0);
                continue;
            }

            case UIMSG_SelectKeyPage1:
                KeyboardPageNum = 1;
                continue;
            case UIMSG_SelectKeyPage2:
                KeyboardPageNum = 2;
                continue;

            case UIMSG_OpenVideoOptions: {
                pMessageQueue_50CBD0->Flush();

                pGUIWindow_CurrentMenu->Release();
                pGUIWindow_CurrentMenu =
                    new GUIWindow_GameVideoOptions();  // GameMenuUI_OptionsVideo_Load();
                viewparams->field_48 = 1;
                current_screen_type = SCREEN_VIDEO_OPTIONS;

                continue;
            }

            case UIMSG_1A9:
                __debugbreak();
                if (param == 4) {
                    // --uGammaPos;
                    if ((uGammaPos-- - 1) < 0) {
                        uGammaPos = 0;
                        pAudioPlayer->PlaySound(SOUND_ClickMovingSelector, 0, 0,
                                                -1, 0, 0, 0, 0);
                        continue;
                    }
                    double v19 = (double)(signed int)uGammaPos * 0.1 + 0.6;
                    // pEngine->pGammaController->Initialize(v19);
                    new OnButtonClick2(21, 161, 0, 0, (int)pBtn_SliderLeft,
                                       String(), false);
                    pAudioPlayer->PlaySound(SOUND_ClickMovingSelector, 0, 0, -1,
                                            0, 0, 0, 0);
                    continue;
                }
                if (param == 5) {
                    ++uGammaPos;
                    if ((signed int)uGammaPos <= 9) {
                        double v21 = (double)(signed int)uGammaPos * 0.1 + 0.6;
                        // pEngine->pGammaController->Initialize(v21);
                        new OnButtonClick2(213, 161, 0, 0,
                                           (int)pBtn_SliderRight, String(),
                                           false);
                        pAudioPlayer->PlaySound(SOUND_ClickMovingSelector, 0, 0,
                                                -1, 0, 0, 0, 0);
                        continue;
                    }
                    uGammaPos = 9;
                } else {
                    Point pt = pMouse->GetCursorPos();
                    uGammaPos = (pt.x - 42) / 17;
                    double v22 = (double)(signed int)uGammaPos * 0.1 + 0.6;
                    // pEngine->pGammaController->Initialize(v22);
                }
                pAudioPlayer->PlaySound(SOUND_ClickMovingSelector, 0, 0, -1, 0,
                                        0, 0, 0);
                continue;
            case UIMSG_ToggleBloodsplats:
                pEngine->ToggleFlags2(0x20u);
                continue;
            case UIMSG_ToggleColoredLights:
                render->ToggleColoredLights();
                continue;
            case UIMSG_ToggleTint:
                render->ToggleTint();
                continue;

            case UIMSG_ChangeMusicVolume: {
                extern bool use_music_folder;
                if (param == 4) {  // -
                    --uMusicVolimeMultiplier;
                    if ((char)uMusicVolimeMultiplier < 1)
                        uMusicVolimeMultiplier = 0;
                    new OnButtonClick2(243, 216, 0, 0, (int)pBtn_SliderLeft,
                                       String(), false);
                    if (uMusicVolimeMultiplier)
                        pAudioPlayer->PlaySound(
                            SOUND_hurp, -1, 0, -1, 0, 0,
                            pSoundVolumeLevels[uMusicVolimeMultiplier] * 64.0f,
                            0);

                    pAudioPlayer->MusicSetVolume(uMusicVolimeMultiplier);

                    continue;
                }
                if (param == 5) {  // +
                    ++uMusicVolimeMultiplier;
                    if ((char)uMusicVolimeMultiplier > 9)
                        uMusicVolimeMultiplier = 9;
                    new OnButtonClick2(435, 216, 0, 0, (int)pBtn_SliderRight,
                                       String(), false);
                    if (uMusicVolimeMultiplier)
                        pAudioPlayer->PlaySound(
                            SOUND_hurp, -1, 0, -1, 0, 0,
                            pSoundVolumeLevels[uMusicVolimeMultiplier] * 64.0f,
                            0);
                    pAudioPlayer->MusicSetVolume(uMusicVolimeMultiplier);
                    continue;
                }

                Point pt = pMouse->GetCursorPos();
                uMusicVolimeMultiplier = (pt.x - 263) / 17;  // for mouse
                if ((char)uMusicVolimeMultiplier > 9)
                    uMusicVolimeMultiplier = 9;
                if (uMusicVolimeMultiplier)
                    pAudioPlayer->PlaySound(
                        SOUND_hurp, -1, 0, -1, 0, 0,
                        pSoundVolumeLevels[uMusicVolimeMultiplier] * 64.0f, 0);
                pAudioPlayer->MusicSetVolume(uMusicVolimeMultiplier);
                continue;
            }

            case UIMSG_ChangeSoundVolume: {
                if (param == 4) {  // reduce sound level button left
                    if (uSoundVolumeMultiplier > 0) --uSoundVolumeMultiplier;

                    new OnButtonClick2(243, 162, 0, 0, (int)pBtn_SliderLeft,
                                       String(), false);
                    pAudioPlayer->SetMasterVolume(
                        pSoundVolumeLevels[uSoundVolumeMultiplier] * 128.0f);
                    pAudioPlayer->PlaySound(SOUND_church, -1, 0, -1, 0, 0, 0,
                                            0);
                    // int v = AIL_redbook_volume(pAudioPlayer->hAILRedbook);
                    // v = v+1;
                    // if (v)
                    //    __debugbreak();
                    continue;
                }
                if (param == 5) {  // Increase sound level button right
                    ++uSoundVolumeMultiplier;
                    if ((char)uSoundVolumeMultiplier > 8)
                        uSoundVolumeMultiplier = 9;
                    // v168 = 1;
                    // v154 = (int)pBtn_SliderRight;
                    new OnButtonClick2(435, 162, 0, 0, (int)pBtn_SliderRight,
                                       String(), false);
                    pAudioPlayer->SetMasterVolume(
                        pSoundVolumeLevels[uSoundVolumeMultiplier] * 128.0f);
                    pAudioPlayer->PlaySound(SOUND_church, -1, 0, -1, 0, 0, 0,
                                            0);
                    continue;
                }

                Point pt = pMouse->GetCursorPos();
                uSoundVolumeMultiplier = (pt.x - 263) / 17;
                if ((char)uSoundVolumeMultiplier > 8)
                    uSoundVolumeMultiplier = 9;
                pAudioPlayer->SetMasterVolume(
                    pSoundVolumeLevels[uSoundVolumeMultiplier] * 128.0f);
                pAudioPlayer->PlaySound(SOUND_church, -1, 0, -1, 0, 0, 0, 0);
                continue;
            }
            case UIMSG_ToggleFlipOnExit:
                bFlipOnExit = bFlipOnExit == 0;
                continue;
            case UIMSG_ToggleAlwaysRun:
                bAlwaysRun = bAlwaysRun == 0;
                continue;
            case UIMSG_ToggleWalkSound:
                bWalkSound = bWalkSound == 0;
                continue;
            case UIMSG_ToggleShowDamage:
                bShowDamage = bShowDamage == 0;
                continue;
            case UIMSG_ChangeVoiceVolume: {
                if (param == 4) {
                    --uVoicesVolumeMultiplier;
                    if ((char)uVoicesVolumeMultiplier < 1)
                        uVoicesVolumeMultiplier = 0;
                    new OnButtonClick2(243, 270, 0, 0, (int)pBtn_SliderLeft,
                                       String(), false);
                    if (!uVoicesVolumeMultiplier) continue;
                    pAudioPlayer->PlaySound(
                        SOUND_hf445a, -1, 0, -1, 0, 0,
                        pSoundVolumeLevels[uVoicesVolumeMultiplier] * 128.0f,
                        0);
                    continue;
                }
                if (param == 5) {
                    ++uVoicesVolumeMultiplier;
                    if ((char)uVoicesVolumeMultiplier > 8)
                        uVoicesVolumeMultiplier = 9;
                    new OnButtonClick2(435, 270, 0, 0, (int)pBtn_SliderRight,
                                       String(), false);
                    if (!uVoicesVolumeMultiplier) continue;
                    pAudioPlayer->PlaySound(
                        SOUND_hf445a, -1, 0, -1, 0, 0,
                        pSoundVolumeLevels[uVoicesVolumeMultiplier] * 128.0f,
                        0);
                    continue;
                }

                Point pt = pMouse->GetCursorPos();
                uVoicesVolumeMultiplier = (pt.x - 263) / 17;
                if ((char)uVoicesVolumeMultiplier > 8)
                    uVoicesVolumeMultiplier = 9;
                if (!uVoicesVolumeMultiplier) continue;
                pAudioPlayer->PlaySound(
                    SOUND_hf445a, -1, 0, -1, 0, 0,
                    pSoundVolumeLevels[uVoicesVolumeMultiplier] * 128.0f, 0);
                continue;
            }
            case UIMSG_SetTurnSpeed:
                if (param)
                    pParty->sRotationY = param * pParty->sRotationY / param;
                uTurnSpeed = param;
                continue;

            case UIMSG_SetGraphicsMode:
                // if ( !bUseLoResSprites )
                //{
                //    byte_6BE388_graphicsmode = uMessageParam;
                //    MM7Initialization();
                //    continue;
                //}
                // if ( uMessageParam )
                //{
                //    if ( uMessageParam == 1 )
                //    {
                //    byte_6BE388_graphicsmode = 0;
                //    }
                //    else
                //    {
                //        if ( uMessageParam != 2 )
                //            continue;
                //        byte_6BE388_graphicsmode = 1;
                //    }
                //    MM7Initialization();
                //    continue;
                //}
                pModalWindow =
                    new GUIWindow_Modal(pNPCTopics[453].pText, UIMSG_0);
                __debugbreak();  // Nomad: graphicsmode as it was now removed
                continue;

            case UIMSG_GameMenu_ReturnToGame:
                // pGUIWindow_CurrentMenu->Release();
                pEventTimer->Resume();
                current_screen_type = SCREEN_GAME;
                viewparams->bRedrawGameUI = true;
                continue;

            case UIMSG_Escape:
                if (pModalWindow) {
                    pModalWindow->Release();
                    pModalWindow = nullptr;
                    continue;
                }
                render->ClearZBuffer(0, 479);
                viewparams->bRedrawGameUI = true;
                viewparams->field_48 = 1;

                if (current_screen_type == SCREEN_MENU) {
                    current_screen_type = SCREEN_GAME;
                } else if (current_screen_type == SCREEN_SAVEGAME ||
                           current_screen_type == SCREEN_LOADGAME) {
                    // crt_deconstruct_ptr_6A0118();

                    pGUIWindow_CurrentMenu->Release();
                    current_screen_type = SCREEN_MENU;
                    pGUIWindow_CurrentMenu = new GUIWindow_GameMenu();
                } else if (current_screen_type == SCREEN_OPTIONS) {
                    options_menu_skin.Relaease();
                    OS_SetAppInt("soundflag", (char)uSoundVolumeMultiplier);
                    OS_SetAppInt("musicflag", (char)uMusicVolimeMultiplier);
                    OS_SetAppInt("CharVoices", (char)uVoicesVolumeMultiplier);
                    OS_SetAppInt("WalkSound", bWalkSound);
                    OS_SetAppInt("ShowDamage", bShowDamage);
                    // OS_SetAppInt("graphicsmode", (unsigned
                    // __int8)byte_6BE388_graphicsmode);
                    OS_SetAppInt("valAlwaysRun", bAlwaysRun);
                    OS_SetAppInt("FlipOnExit", bFlipOnExit);
                    if (!uTurnSpeed)
                        OS_SetAppInt("TurnDelta", 3);
                    else if (uTurnSpeed == 64)
                        OS_SetAppInt("TurnDelta", 2);
                    else if (uTurnSpeed == 128)
                        OS_SetAppInt("TurnDelta", 1);

                    pGUIWindow_CurrentMenu->Release();
                    current_screen_type = SCREEN_MENU;
                    pGUIWindow_CurrentMenu = new GUIWindow_GameMenu();
                } else if (current_screen_type == SCREEN_VIDEO_OPTIONS) {
                    // if ( render->pRenderD3D )
                    {
                        OS_SetAppInt("Colored Lights",
                                     render->bUseColoredLights);
                        OS_SetAppInt("Tinting", render->bTinting);
                        OS_SetAppInt("Bloodsplats",
                                     ((pEngine->uFlags2 & 0xFF) >> 5) & 1);
                    }

                    pGUIWindow_CurrentMenu->Release();
                    current_screen_type = SCREEN_MENU;
                    pGUIWindow_CurrentMenu = new GUIWindow_GameMenu();
                } else if (current_screen_type == SCREEN_KEYBOARD_OPTIONS) {
                    KeyToggleType pKeyToggleType;  // [sp+0h] [bp-5FCh]@287
                    int v197 = 1;
                    bool pKeyBindingFlag = false;

                    for (uint i = 0; i < 28; ++i) {
                        if (GameMenuUI_InvaligKeyBindingsFlags[i])
                            pKeyBindingFlag = true;
                    }
                    if (!pKeyBindingFlag) {
                        for (uint i = 0; i < 5; i++) {
                            if (game_ui_options_controls[i]) {
                                game_ui_options_controls[i]->Release();
                                game_ui_options_controls[i] = nullptr;
                            }
                        }

                        for (uint i = 0; i < 28; ++i) {
                            if (pKeyActionMap->GetActionVKey(
                                    (enum InputAction)i) !=
                                pPrevVirtualCidesMapping[i]) {
                                if (v197) {
                                    GUI_ReplaceHotkey(
                                        pKeyActionMap->GetActionVKey(
                                            (enum InputAction)i),
                                        (unsigned char)
                                            pPrevVirtualCidesMapping[i],
                                        1);
                                    v197 = 0;
                                } else {
                                    GUI_ReplaceHotkey(
                                        pKeyActionMap->GetActionVKey(
                                        (enum InputAction)i),
                                            (unsigned char)
                                        pPrevVirtualCidesMapping[i],
                                        0);
                                }
                            }
                            if (i > 3 && i != 25 && i != 26)
                                pKeyToggleType = TOGGLE_OneTimePress;
                            else
                                pKeyToggleType = TOGGLE_Continuously;
                            pKeyActionMap->SetKeyMapping(
                                i, pPrevVirtualCidesMapping[i], pKeyToggleType);
                        }
                        pKeyActionMap->StoreMappings();
                    } else {
                        pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0, 0,
                            0);
                    }

                    pGUIWindow_CurrentMenu->Release();
                    current_screen_type = SCREEN_MENU;
                    pGUIWindow_CurrentMenu = new GUIWindow_GameMenu();
                }
                continue;
        }
    }
}

void GameMenu_Loop() {
    pEventTimer->Pause();
    pAudioPlayer->StopChannels(-1, -1);
    current_screen_type = SCREEN_MENU;

    pGUIWindow_CurrentMenu = new GUIWindow_GameMenu();

    viewparams->field_48 = 1;

    if (gamma_preview_image) {
        gamma_preview_image->Release();
        gamma_preview_image = nullptr;
    }

    render->SaveScreenshot("gamma.pcx", 155, 117);
    gamma_preview_image = assets->GetImage_PCXFromFile("gamma.pcx");

    while (uGameState == GAME_STATE_PLAYING &&
           (current_screen_type == SCREEN_MENU ||
            current_screen_type == SCREEN_SAVEGAME ||
            current_screen_type == SCREEN_LOADGAME ||
            current_screen_type == SCREEN_OPTIONS ||
            current_screen_type == SCREEN_VIDEO_OPTIONS ||
            current_screen_type == SCREEN_KEYBOARD_OPTIONS)) {
        OS_PeekMessageLoop();
        if (dword_6BE364_game_settings_1 & GAME_SETTINGS_APP_INACTIVE) {
            OS_WaitMessage();
            continue;
        }

        render->BeginScene();
        {
            GameMenu_EventLoop();
            GUI_UpdateWindows();
        }
        render->EndScene();
        render->Present();
    }

    pGUIWindow_CurrentMenu->Release();
    pGUIWindow_CurrentMenu = nullptr;

    if (gamma_preview_image) {
        gamma_preview_image->Release();
        gamma_preview_image = nullptr;
    }
}
