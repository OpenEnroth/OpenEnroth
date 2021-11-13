#include <map>

#include "src/Application/GameMenu.h"

#include "src/Application/Game.h"

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/SaveLoad.h"
#include "Engine/Time.h"

#include "Io/GameKey.h"
#include "Io/InputAction.h"
#include "Io/KeyboardInputHandler.h"
#include "Io/Mouse.h"

#include "GUI/GUIButton.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIModal.h"
#include "GUI/UI/UISaveLoad.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

#include "Platform/Api.h"
#include "Platform/OSWindow.h"


using Application::Menu;
using Io::TextInputType;
using Io::KeyToggleType;
using Io::InputAction;


InputAction currently_selected_action_for_binding = InputAction::Invalid;  // 506E68
std::map<InputAction, bool> key_map_conflicted;  // 506E6C
std::map<InputAction, GameKey> prev_key_map;

void Game_StartNewGameWhilePlaying(bool force_start) {
    if (dword_6BE138 == 124 || force_start) {
        pMessageQueue_50CBD0->Flush();
        // pGUIWindow_CurrentMenu->Release();
        uGameState = GAME_STATE_NEWGAME_OUT_GAMEMENU;
        current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
        viewparams->bRedrawGameUI = 1;
    } else {
        GameUI_StatusBar_OnEvent(localization->GetString(
            201));  // "Are you sure?  Click again to start a New Game"
        pAudioPlayer->PlaySound(SOUND_quest, 0, 0, -1, 0, 0);
        dword_6BE138 = 124;
    }
}

void Game_QuitGameWhilePlaying(bool force_quit) {
    if (dword_6BE138 == 132 || force_quit) {
        pMessageQueue_50CBD0->Flush();
        // pGUIWindow_CurrentMenu->Release();
        current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
        viewparams->bRedrawGameUI = 1;
        pAudioPlayer->PlaySound(SOUND_WoodDoorClosing, 0, 0, -1, 0, 0);
        uGameState = GAME_STATE_GAME_QUITTING_TO_MAIN_MENU;
    } else {
        GameUI_StatusBar_OnEvent(localization->GetString(
            82));  // "Are you sure?  Click again to quit"
        pAudioPlayer->PlaySound(SOUND_quest, 0, 0, -1, 0, 0);
        dword_6BE138 = 132;
    }
}

void Game_OpenLoadGameDialog() {
    pMessageQueue_50CBD0->Flush();
    pGUIWindow_CurrentMenu->Release();
    pGUIWindow_CurrentMenu = nullptr;
    game_ui_status_bar_event_string_time_left = 0;
    // LoadUI_Load(1);
    current_screen_type = CURRENT_SCREEN::SCREEN_LOADGAME;
    pGUIWindow_CurrentMenu = new GUIWindow_Load(true);
}

void Menu::EventLoop() {
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
                new OnButtonClick2(215, 199, 17, 17, pBtnArrowUp);
                continue;

            case UIMSG_DownArrow:
                ++pSaveListPosition;
                if (pSaveListPosition > (param - 7) ) pSaveListPosition = (param - 7);
                new OnButtonClick2(215, 323, 17, 17, pBtnDownArrow);
                continue;

            case UIMSG_Cancel:
                new OnCancel(350, 302, 106, 42, pBtnCancel);
                continue;

            case UIMSG_SaveLoadBtn:
                new OnSaveLoad(241, 302, 106, 42, pBtnLoadSlot);
                continue;
            case UIMSG_SelectLoadSlot: {
                if (pGUIWindow_CurrentMenu->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_IN_PROGRESS)
                    keyboardInputHandler->SetWindowInputStatus(WindowInputStatus::WINDOW_INPUT_NONE);

                int v10 = pSaveListPosition + param;
                if (current_screen_type != CURRENT_SCREEN::SCREEN_SAVEGAME ||
                    uLoadGameUI_SelectedSlot != v10) {
                    if (dword_6BE138 == pSaveListPosition + param) {
                        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_SaveLoadBtn, 0, 0);
                        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_LoadGame, 0, 0);
                    }
                    uLoadGameUI_SelectedSlot = v10;
                    dword_6BE138 = v10;
                } else {
                    keyboardInputHandler->StartTextInput(TextInputType::Text, 19, pGUIWindow_CurrentMenu);
                    if (strcmp(pSavegameHeader[uLoadGameUI_SelectedSlot].pName, localization->GetString(LSTR_EMPTY_SAVESLOT))) {
                        keyboardInputHandler->SetTextInput(pSavegameHeader[uLoadGameUI_SelectedSlot].pName);
                    }
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
                if (pGUIWindow_CurrentMenu->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_IN_PROGRESS) {
                    keyboardInputHandler->SetWindowInputStatus(WindowInputStatus::WINDOW_INPUT_NONE);
                    strcpy(pSavegameHeader[uLoadGameUI_SelectedSlot].pName, keyboardInputHandler->GetTextInput().c_str());
                }
                DoSavegame(uLoadGameUI_SelectedSlot);
                continue;
            case UIMSG_Game_OpenSaveGameDialog: {
                pGUIWindow_CurrentMenu->Release();
                game_ui_status_bar_event_string_time_left = 0;
                current_screen_type = CURRENT_SCREEN::SCREEN_SAVEGAME;
                pGUIWindow_CurrentMenu = new GUIWindow_Save();
                // SaveUI_Load(current_screen_type = SCREEN_SAVEGAME);
                continue;
            }
            case UIMSG_Game_OpenOptionsDialog:  // Open
            {
                pMessageQueue_50CBD0->Flush();

                pGUIWindow_CurrentMenu->Release();
                pGUIWindow_CurrentMenu = new GUIWindow_GameOptions();  // GameMenuUI_Options_Load();

                viewparams->field_48 = 1;
                current_screen_type = CURRENT_SCREEN::SCREEN_OPTIONS;

                continue;
            }

            case UIMSG_OpenKeyMappingOptions:  // Open
            {
                pMessageQueue_50CBD0->Flush();

                pGUIWindow_CurrentMenu->Release();
                pGUIWindow_CurrentMenu = new GUIWindow_GameKeyBindings();  // GameMenuUI_OptionsKeymapping_Load();

                viewparams->field_48 = 1;
                current_screen_type = CURRENT_SCREEN::SCREEN_KEYBOARD_OPTIONS;

                continue;
            }

            case UIMSG_ChangeKeyButton: {
                if (currently_selected_action_for_binding != InputAction::Invalid) {
                    pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
                } else {
                    currently_selected_action_for_binding = (InputAction)param;
                    if (KeyboardPageNum != 1)
                        currently_selected_action_for_binding = (InputAction)(param + 14);
                    keyboardInputHandler->StartTextInput(TextInputType::Text, 1, pGUIWindow_CurrentMenu);
                }
                continue;
            }

            case UIMSG_ResetKeyMapping: {
                int v197 = 1;
                // keyboardController->SetDefaultMapping();
                for (auto action : AllInputActions()) {
                    GameKey oldKey = keyboardActionMapping->GetKey(action);
                    GameKey newKey = keyboardActionMapping->MapDefaultKey(action);

                    if (oldKey != newKey) {
                        GUI_ReplaceHotkey(oldKey, newKey, v197);
                        v197 = 0;
                    }
                    prev_key_map[action] = oldKey;
                    key_map_conflicted[action] = false;
                }
                pAudioPlayer->PlaySound(SOUND_chimes, 0, 0, -1, 0, 0);
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
                pGUIWindow_CurrentMenu = new GUIWindow_GameVideoOptions();  // GameMenuUI_OptionsVideo_Load();
                viewparams->field_48 = 1;
                current_screen_type = CURRENT_SCREEN::SCREEN_VIDEO_OPTIONS;

                continue;
            }

            case UIMSG_1A9:
                __debugbreak();
                if (param == 4) {
                    // --uGammaPos;
                    if ((uGammaPos-- - 1) < 0) {
                        uGammaPos = 0;
                        pAudioPlayer->PlaySound(SOUND_ClickMovingSelector, 0, 0, -1, 0, 0);
                        continue;
                    }
                    double v19 = (double)(signed int)uGammaPos * 0.1 + 0.6;
                    // engine->pGammaController->Initialize(v19);
                    new OnButtonClick2(21, 161, 0, 0, pBtn_SliderLeft,
                                       String(), false);
                    pAudioPlayer->PlaySound(SOUND_ClickMovingSelector, 0, 0, -1, 0, 0);
                    continue;
                }
                if (param == 5) {
                    ++uGammaPos;
                    if ((signed int)uGammaPos <= 9) {
                        double v21 = (double)(signed int)uGammaPos * 0.1 + 0.6;
                        // engine->pGammaController->Initialize(v21);
                        new OnButtonClick2(213, 161, 0, 0,
                                           pBtn_SliderRight, String(),
                                           false);
                        pAudioPlayer->PlaySound(SOUND_ClickMovingSelector, 0, 0, -1, 0, 0);
                        continue;
                    }
                    uGammaPos = 9;
                } else {
                    Point pt = mouse->GetCursorPos();
                    uGammaPos = (pt.x - 42) / 17;
                    double v22 = (double)(signed int)uGammaPos * 0.1 + 0.6;
                    // engine->pGammaController->Initialize(v22);
                }
                pAudioPlayer->PlaySound(SOUND_ClickMovingSelector, 0, 0, -1, 0, 0);
                continue;
            case UIMSG_ToggleBloodsplats:
                engine->ToggleBloodsplats();
                continue;
            case UIMSG_ToggleColoredLights:
                render->ToggleColoredLights();
                continue;
            case UIMSG_ToggleTint:
                render->ToggleTint();
                continue;

            case UIMSG_ChangeMusicVolume: {
                int new_level = engine->config->music_level;
                if (param == 4) {
                    new_level -= 1;
                    new OnButtonClick2(243, 216, 0, 0, pBtn_SliderLeft, String(), false);
                } else if (param == 5) {
                    new_level += 1;
                    new OnButtonClick2(435, 216, 0, 0, pBtn_SliderRight, String(), false);
                } else {
                    Point pt = mouse->GetCursorPos();
                    new_level = (pt.x - 263) / 17;  // for mouse
                }

                engine->SetMusicLevel(new_level);
                pAudioPlayer->SetMusicVolume(engine->config->music_level);
                // if (engine->config->music_level > 0)
                //    pAudioPlayer->PlaySound(SOUND_hurp, -1, 0, -1, 0, 0);
                continue;
            }

            case UIMSG_ChangeSoundVolume: {
                int new_level = engine->config->sound_level;
                if (param == 4) {
                    new_level -= 1;
                    new OnButtonClick2(243, 162, 0, 0, pBtn_SliderLeft, String(), false);
                } else if (param == 5) {
                    new_level += 1;
                    new OnButtonClick2(435, 162, 0, 0, pBtn_SliderRight, String(), false);
                } else {
                    Point pt = mouse->GetCursorPos();
                    new_level = (pt.x - 263) / 17;
                }

                engine->SetSoundLevel(new_level);

                pAudioPlayer->SetMasterVolume(engine->config->sound_level);
                pAudioPlayer->PlaySound(SOUND_church, -1, 0, -1, 0, 0);
                continue;
            }
            case UIMSG_ToggleFlipOnExit:
                engine->ToggleFlipOnExit();
                continue;
            case UIMSG_ToggleAlwaysRun:
                engine->ToggleAlwaysRun();
                continue;
            case UIMSG_ToggleWalkSound:
                engine->ToggleWalkSound();
                continue;
            case UIMSG_ToggleShowDamage:
                engine->ToggleShowDamage();
                continue;
            case UIMSG_ChangeVoiceVolume: {
                int new_level = engine->config->voice_level;
                if (param == 4) {
                    new_level -= 1;
                    new OnButtonClick2(243, 270, 0, 0, pBtn_SliderLeft, String(), false);
                } else if (param == 5) {
                    new_level += 1;
                    new OnButtonClick2(435, 270, 0, 0, pBtn_SliderRight, String(), false);
                } else {
                    Point pt = mouse->GetCursorPos();
                    new_level = (pt.x - 263) / 17;
                }

                engine->SetVoiceLevel(new_level);
                if (engine->config->voice_level > 0)
                    pAudioPlayer->PlaySound(SOUND_hf445a, 44, 0, -1, 0, 0);
                continue;
            }
            case UIMSG_SetTurnSpeed:
                if (param)
                    pParty->sRotationZ = param * pParty->sRotationZ / param;
                engine->SetTurnSpeed(param);
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
                current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
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

                if (current_screen_type == CURRENT_SCREEN::SCREEN_MENU) {
                    pEventTimer->Resume();
                    current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
                    viewparams->bRedrawGameUI = true;
                } else if (current_screen_type == CURRENT_SCREEN::SCREEN_SAVEGAME ||
                           current_screen_type == CURRENT_SCREEN::SCREEN_LOADGAME) {
                    // crt_deconstruct_ptr_6A0118();

                    pGUIWindow_CurrentMenu->Release();
                    current_screen_type = CURRENT_SCREEN::SCREEN_MENU;
                    pGUIWindow_CurrentMenu = new GUIWindow_GameMenu();
                } else if (current_screen_type == CURRENT_SCREEN::SCREEN_OPTIONS) {
                    options_menu_skin.Relaease();
                    OS_SetAppInt("soundflag", engine->config->sound_level);
                    OS_SetAppInt("musicflag", engine->config->music_level);
                    OS_SetAppInt("CharVoices", engine->config->voice_level);
                    OS_SetAppInt("WalkSound", engine->config->NoWalkSound() ? 0 : 1);
                    OS_SetAppInt("ShowDamage", engine->config->NoShowDamage() ? 0 : 1);
                    // OS_SetAppInt("graphicsmode", (unsigned
                    // __int8)byte_6BE388_graphicsmode);
                    OS_SetAppInt("valAlwaysRun", engine->config->always_run ? 1 : 0);
                    OS_SetAppInt("FlipOnExit", engine->config->flip_on_exit ? 1 : 0);

                    if (engine->config->turn_speed == 0)
                        OS_SetAppInt("TurnDelta", 3);
                    else if (engine->config->turn_speed == 64)
                        OS_SetAppInt("TurnDelta", 2);
                    else if (engine->config->turn_speed == 128)
                        OS_SetAppInt("TurnDelta", 1);

                    pGUIWindow_CurrentMenu->Release();
                    current_screen_type = CURRENT_SCREEN::SCREEN_MENU;
                    pGUIWindow_CurrentMenu = new GUIWindow_GameMenu();
                } else if (current_screen_type == CURRENT_SCREEN::SCREEN_VIDEO_OPTIONS) {
                    // if ( render->pRenderD3D )
                    {
                        OS_SetAppInt("Colored Lights", render->config->is_using_colored_lights);
                        OS_SetAppInt("Tinting", render->config->is_tinting);
                        OS_SetAppInt("Bloodsplats", engine->config->NoBloodsplats() ? 0 : 1);
                    }

                    pGUIWindow_CurrentMenu->Release();
                    current_screen_type = CURRENT_SCREEN::SCREEN_MENU;
                    pGUIWindow_CurrentMenu = new GUIWindow_GameMenu();
                } else if (current_screen_type == CURRENT_SCREEN::SCREEN_KEYBOARD_OPTIONS) {
                    // KeyToggleType pKeyToggleType;  // [sp+0h] [bp-5FCh]@287
                    int v197 = 1;
                    bool anyBindingErrors = false;

                    for (auto action : AllInputActions()) {
                        if (key_map_conflicted[action]) {
                            anyBindingErrors = true;
                        }
                    }
                    if (anyBindingErrors) {
                        pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
                    } else {
                        for (uint i = 0; i < 5; i++) {
                            if (game_ui_options_controls[i]) {
                                game_ui_options_controls[i]->Release();
                                game_ui_options_controls[i] = nullptr;
                            }
                        }

                        for (auto action : AllInputActions()) {
                            if (keyboardActionMapping->GetKey(action) != prev_key_map[action]) {
                                GUI_ReplaceHotkey(
                                    keyboardActionMapping->GetKey(action),
                                    prev_key_map[action],
                                    v197);
                                v197 = 0;
                            }

                            keyboardActionMapping->MapKey(action, prev_key_map[action], GetToggleType(action));
                        }
                        keyboardActionMapping->StoreMappings();
                    }

                    pGUIWindow_CurrentMenu->Release();
                    current_screen_type = CURRENT_SCREEN::SCREEN_MENU;
                    pGUIWindow_CurrentMenu = new GUIWindow_GameMenu();
                }
                continue;
        }
    }
}

void Menu::MenuLoop() {
    pEventTimer->Pause();
    pAudioPlayer->StopChannels(-1, -1);
    current_screen_type = CURRENT_SCREEN::SCREEN_MENU;

    pGUIWindow_CurrentMenu = new GUIWindow_GameMenu();

    viewparams->field_48 = 1;

    if (gamma_preview_image) {
        gamma_preview_image->Release();
        gamma_preview_image = nullptr;
    }

    render->SaveScreenshot("gamma.pcx", 155, 117);
    gamma_preview_image = assets->GetImage_PCXFromFile("gamma.pcx");

    while (uGameState == GAME_STATE_PLAYING &&
           (current_screen_type == CURRENT_SCREEN::SCREEN_MENU ||
            current_screen_type == CURRENT_SCREEN::SCREEN_SAVEGAME ||
            current_screen_type == CURRENT_SCREEN::SCREEN_LOADGAME ||
            current_screen_type == CURRENT_SCREEN::SCREEN_OPTIONS ||
            current_screen_type == CURRENT_SCREEN::SCREEN_VIDEO_OPTIONS ||
            current_screen_type == CURRENT_SCREEN::SCREEN_KEYBOARD_OPTIONS)) {
        MessageLoopWithWait();

        GameUI_WritePointedObjectStatusString();
        render->BeginScene();
        {
            EventLoop();
            engine->DrawGUI();
            GUI_UpdateWindows();
            GameUI_StatusBar_Draw();
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
