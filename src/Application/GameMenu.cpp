#include <map>
#include <string>

#include "GameMenu.h"

#include "Game.h"

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/SaveLoad.h"
#include "Engine/Time.h"

#include "Platform/PlatformEnums.h"
#include "Io/InputAction.h"
#include "Io/KeyboardInputHandler.h"
#include "Io/Mouse.h"

#include "GUI/GUIButton.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIGameOver.h"
#include "GUI/UI/UISaveLoad.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"



using Application::Menu;
using Io::TextInputType;
using Io::KeyToggleType;
using Io::InputAction;


InputAction currently_selected_action_for_binding = InputAction::Invalid;  // 506E68
std::map<InputAction, bool> key_map_conflicted;  // 506E6C
std::map<InputAction, PlatformKey> curr_key_map;

void Game_StartNewGameWhilePlaying(bool force_start) {
    if (dword_6BE138 == 124 || force_start) {
        pCurrentFrameMessageQueue->Flush();
        // pGUIWindow_CurrentMenu->Release();
        uGameState = GAME_STATE_NEWGAME_OUT_GAMEMENU;
        current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
    } else {
        GameUI_SetStatusBar(LSTR_START_NEW_GAME_PROMPT);
        pAudioPlayer->PlaySound(SOUND_quest, 0, 0, -1, 0, 0);
        dword_6BE138 = 124;
    }
}

void Game_QuitGameWhilePlaying(bool force_quit) {
    if (dword_6BE138 == 132 || force_quit) {
        pCurrentFrameMessageQueue->Flush();
        // pGUIWindow_CurrentMenu->Release();
        current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
        pAudioPlayer->PlaySound(SOUND_WoodDoorClosing, 0, 0, -1, 0, 0);
        uGameState = GAME_STATE_GAME_QUITTING_TO_MAIN_MENU;
    } else {
        GameUI_SetStatusBar(LSTR_EXIT_GAME_PROMPT);
        pAudioPlayer->PlaySound(SOUND_quest, 0, 0, -1, 0, 0);
        dword_6BE138 = 132;
    }
}

void Game_OpenLoadGameDialog() {
    pCurrentFrameMessageQueue->Flush();
    pGUIWindow_CurrentMenu->Release();
    pGUIWindow_CurrentMenu = nullptr;
    game_ui_status_bar_event_string_time_left = 0;
    // LoadUI_Load(1);
    current_screen_type = CURRENT_SCREEN::SCREEN_LOADGAME;
    pGUIWindow_CurrentMenu = new GUIWindow_Load(true);
}

void Menu::EventLoop() {
    while (!pCurrentFrameMessageQueue->Empty()) {
        UIMessageType msg;
        int param, param2;
        pCurrentFrameMessageQueue->PopMessage(&msg, &param, &param2);

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
                new OnButtonClick2({215, 199}, {17, 17}, pBtnArrowUp);
                continue;

            case UIMSG_DownArrow:
                ++pSaveListPosition;
                if (pSaveListPosition > (param - 7) ) pSaveListPosition = (param - 7);
                new OnButtonClick2({215, 323}, {17, 17}, pBtnDownArrow);
                continue;

            case UIMSG_Cancel:
                new OnCancel({350, 302}, {106, 42}, pBtnCancel);
                continue;

            case UIMSG_SaveLoadBtn:
                new OnSaveLoad({241, 302}, {106, 42}, pBtnLoadSlot);
                continue;
            case UIMSG_SelectLoadSlot: {
                if (pGUIWindow_CurrentMenu->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_IN_PROGRESS)
                    keyboardInputHandler->SetWindowInputStatus(WindowInputStatus::WINDOW_INPUT_NONE);

                int v10 = pSaveListPosition + param;
                if (current_screen_type != CURRENT_SCREEN::SCREEN_SAVEGAME ||
                    uLoadGameUI_SelectedSlot != v10) {
                    if (dword_6BE138 == pSaveListPosition + param) {
                        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_SaveLoadBtn, 0, 0);
                        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_LoadGame, 0, 0);
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
                    // TODO(pskelton): Move Reset() inside LoadGame at all call sites
                    pParty->Reset();
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
            case UIMSG_SaveLoadScroll: {
                // pskelton add for scroll click
                int pSaveFiles{ static_cast<int>(uNumSavegameFiles) };
                int mx{}, my{};
                mouse->GetClickPos(&mx, &my);
                // 216 is offset down from top (216)
                my -= 216;
                // 107 is total height of bar
                float fmy = static_cast<float>(my) / 107.0f;
                int newlistpost = std::round((param - 7) * fmy);
                newlistpost = std::clamp(newlistpost, 0, (param - 7));
                pSaveListPosition = newlistpost;
                new OnButtonClick2({pGUIWindow_CurrentMenu->uFrameX + 215, pGUIWindow_CurrentMenu->uFrameY + 197}, {0, 0}, pBtnArrowUp);
                continue;
            }
            case UIMSG_Game_OpenOptionsDialog:  // Open
            {
                pCurrentFrameMessageQueue->Flush();

                pGUIWindow_CurrentMenu->Release();
                pGUIWindow_CurrentMenu = new GUIWindow_GameOptions();  // GameMenuUI_Options_Load();

                current_screen_type = CURRENT_SCREEN::SCREEN_OPTIONS;

                continue;
            }

            case UIMSG_OpenKeyMappingOptions:  // Open
            {
                pCurrentFrameMessageQueue->Flush();

                pGUIWindow_CurrentMenu->Release();
                pGUIWindow_CurrentMenu = new GUIWindow_GameKeyBindings();  // GameMenuUI_OptionsKeymapping_Load();

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
                for (InputAction action : VanillaInputActions()) {
                    PlatformKey newKey = keyboardActionMapping->MapDefaultKey(action);

                    curr_key_map[action] = newKey;
                    key_map_conflicted[action] = false;
                    keyboardActionMapping->MapKey(action, newKey, GetToggleType(action));
                }
                keyboardActionMapping->StoreMappings();
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
                pCurrentFrameMessageQueue->Flush();

                pGUIWindow_CurrentMenu->Release();
                pGUIWindow_CurrentMenu = new GUIWindow_GameVideoOptions();
                current_screen_type = CURRENT_SCREEN::SCREEN_VIDEO_OPTIONS;

                continue;
            }

            case UIMSG_ChangeGammaLevel: {
                int gammalevel = engine->config->graphics.Gamma.Get();
                if (param == 4) {
                    gammalevel--;
                    new OnButtonClick2({21, 161}, {0, 0}, pBtn_SliderLeft, std::string(), false);
                } else if (param == 5) {
                    gammalevel++;
                    new OnButtonClick2({213, 161}, {0, 0}, pBtn_SliderRight, std::string(), false);
                } else {
                    Pointi pt = mouse->GetCursorPos();
                    gammalevel = (pt.x - 42) / 17;
                }

                engine->config->graphics.Gamma.Set(gammalevel);
                pAudioPlayer->PlaySound(SOUND_ClickMovingSelector, 0, 0, -1, 0, 0);

                if (gamma_preview_image) {
                    gamma_preview_image->Release();
                    gamma_preview_image = nullptr;
                }

                render->SaveScreenshot("gamma.pcx", 155, 117);
                gamma_preview_image = assets->GetImage_PCXFromFile("gamma.pcx");


                continue;
                }
            case UIMSG_ToggleBloodsplats:
                engine->config->graphics.BloodSplats.Toggle();
                continue;
            case UIMSG_ToggleColoredLights:
                engine->config->graphics.ColoredLights.Toggle();
                continue;
            case UIMSG_ToggleTint:
                engine->config->graphics.Tinting.Toggle();
                continue;

            case UIMSG_ChangeMusicVolume: {
                int new_level = engine->config->settings.MusicLevel.Get();
                if (param == 4) {
                    new_level -= 1;
                    new OnButtonClick2({243, 216}, {0, 0}, pBtn_SliderLeft, std::string(), false);
                } else if (param == 5) {
                    new_level += 1;
                    new OnButtonClick2({435, 216}, {0, 0}, pBtn_SliderRight, std::string(), false);
                } else {
                    Pointi pt = mouse->GetCursorPos();
                    new_level = (pt.x - 263) / 17;  // for mouse
                }

                engine->config->settings.MusicLevel.Set(new_level);
                pAudioPlayer->SetMusicVolume(engine->config->settings.MusicLevel.Get());
                // if (engine->config->music_level > 0)
                //    pAudioPlayer->PlaySound(SOUND_hurp, PID_INVALID, 0, -1, 0, 0);
                continue;
            }

            case UIMSG_ChangeSoundVolume: {
                int new_level = engine->config->settings.SoundLevel.Get();
                if (param == 4) {
                    new_level -= 1;
                    new OnButtonClick2({243, 162}, {0, 0}, pBtn_SliderLeft, std::string(), false);
                } else if (param == 5) {
                    new_level += 1;
                    new OnButtonClick2({435, 162}, {0, 0}, pBtn_SliderRight, std::string(), false);
                } else {
                    Pointi pt = mouse->GetCursorPos();
                    new_level = (pt.x - 263) / 17;
                }

                engine->config->settings.SoundLevel.Set(new_level);

                pAudioPlayer->SetMasterVolume(engine->config->settings.SoundLevel.Get());
                pAudioPlayer->PlaySound(SOUND_church, PID_INVALID, 0, -1, 0, 0);
                continue;
            }
            case UIMSG_ToggleFlipOnExit:
                engine->config->settings.FlipOnExit.Toggle();
                continue;
            case UIMSG_ToggleAlwaysRun:
                engine->config->settings.AlwaysRun.Toggle();
                continue;
            case UIMSG_ToggleWalkSound:
                engine->config->settings.WalkSound.Toggle();
                continue;
            case UIMSG_ToggleShowDamage:
                engine->config->settings.ShowHits.Toggle();
                continue;
            case UIMSG_ChangeVoiceVolume: {
                int new_level = engine->config->settings.VoiceLevel.Get();
                if (param == 4) {
                    new_level -= 1;
                    new OnButtonClick2({243, 270}, {0, 0}, pBtn_SliderLeft, std::string(), false);
                } else if (param == 5) {
                    new_level += 1;
                    new OnButtonClick2({435, 270}, {0, 0}, pBtn_SliderRight, std::string(), false);
                } else {
                    Pointi pt = mouse->GetCursorPos();
                    new_level = (pt.x - 263) / 17;
                }

                engine->config->settings.VoiceLevel.Set(new_level);
                pAudioPlayer->SetVoiceVolume(engine->config->settings.VoiceLevel.Get());
                if (engine->config->settings.VoiceLevel.Get() > 0)
                    pAudioPlayer->PlaySound(SOUND_hf445a, 44, 0, -1, 0, 0);
                continue;
            }
            case UIMSG_SetTurnSpeed:
                if (param)
                    pParty->sRotationZ = param * pParty->sRotationZ / param;
                engine->config->settings.TurnSpeed.Set(param);
                continue;

            case UIMSG_SetGraphicsMode:
                __debugbreak();  // Nomad: graphicsmode as it was now removed
                continue;

            case UIMSG_GameMenu_ReturnToGame:
                // pGUIWindow_CurrentMenu->Release();
                pEventTimer->Resume();
                current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
                continue;

            case UIMSG_Escape:
                if (pGameOverWindow) {
                    pGameOverWindow->Release();
                    pGameOverWindow = nullptr;
                    continue;
                }
                render->ClearZBuffer();

                if (current_screen_type == CURRENT_SCREEN::SCREEN_MENU) {
                    pEventTimer->Resume();
                    current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
                } else if (current_screen_type == CURRENT_SCREEN::SCREEN_SAVEGAME ||
                           current_screen_type == CURRENT_SCREEN::SCREEN_LOADGAME) {
                    // crt_deconstruct_ptr_6A0118();

                    pGUIWindow_CurrentMenu->Release();
                    current_screen_type = CURRENT_SCREEN::SCREEN_MENU;
                    pGUIWindow_CurrentMenu = new GUIWindow_GameMenu();
                } else if (current_screen_type == CURRENT_SCREEN::SCREEN_OPTIONS) {
                    options_menu_skin.Release();
                    pGUIWindow_CurrentMenu->Release();
                    current_screen_type = CURRENT_SCREEN::SCREEN_MENU;
                    pGUIWindow_CurrentMenu = new GUIWindow_GameMenu();
                } else if (current_screen_type == CURRENT_SCREEN::SCREEN_VIDEO_OPTIONS) {
                    pGUIWindow_CurrentMenu->Release();
                    current_screen_type = CURRENT_SCREEN::SCREEN_MENU;
                    pGUIWindow_CurrentMenu = new GUIWindow_GameMenu();
                } else if (current_screen_type == CURRENT_SCREEN::SCREEN_KEYBOARD_OPTIONS) {
                    // KeyToggleType pKeyToggleType;  // [sp+0h] [bp-5FCh]@287
                    int v197 = 1;
                    bool anyBindingErrors = false;

                    for (auto action : VanillaInputActions()) {
                        if (key_map_conflicted[action]) {
                            anyBindingErrors = true;
                            continue;
                        }
                    }
                    if (anyBindingErrors) {
                        pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
                        break; // deny to exit options until all key conflicts are solved
                    } else {
                        for (uint i = 0; i < 5; i++) {
                            if (game_ui_options_controls[i]) {
                                game_ui_options_controls[i]->Release();
                                game_ui_options_controls[i] = nullptr;
                            }
                        }

                        for (auto action : VanillaInputActions()) {
                            keyboardActionMapping->MapKey(action, curr_key_map[action], GetToggleType(action));
                        }
                        keyboardActionMapping->StoreMappings();
                    }

                    pGUIWindow_CurrentMenu->Release();
                    current_screen_type = CURRENT_SCREEN::SCREEN_MENU;
                    pGUIWindow_CurrentMenu = new GUIWindow_GameMenu();
                }
                continue;

            default:
                break;
        }
    }
}

void Menu::MenuLoop() {
    pEventTimer->Pause();
    pAudioPlayer->PauseSounds(-1);
    current_screen_type = CURRENT_SCREEN::SCREEN_MENU;

    pGUIWindow_CurrentMenu = new GUIWindow_GameMenu();

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
        render->BeginScene2D();
        engine->DrawGUI();
        GUI_UpdateWindows();
        GameUI_StatusBar_Draw();
        render->Present();

        EventLoop();
    }

    pGUIWindow_CurrentMenu->Release();
    pGUIWindow_CurrentMenu = nullptr;

    if (gamma_preview_image) {
        gamma_preview_image->Release();
        gamma_preview_image = nullptr;
    }

    pAudioPlayer->ResumeSounds();
}
