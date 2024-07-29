#include <map>
#include <string>

#include "GameMenu.h"

#include "Game.h"

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/SaveLoad.h"
#include "Engine/Time/Timer.h"
#include "Engine/EngineIocContainer.h"

#include "Library/Platform/Interface/PlatformEnums.h"
#include "Io/InputAction.h"
#include "Io/KeyboardInputHandler.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UISaveLoad.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

using Io::TextInputType;
using Io::KeyToggleType;
using Io::InputAction;

enum class CurrentConfirmationState {
    CONFIRM_NONE,
    CONFIRM_NEW_GAME,
    CONFIRM_QUIT
};
using enum CurrentConfirmationState;

// TODO(Nik-RE-dev): drop variable and load game only on double click
static bool isLoadSlotClicked = false;

CurrentConfirmationState confirmationState = CONFIRM_NONE;

InputAction currently_selected_action_for_binding = Io::InputAction::Invalid;  // 506E68
std::map<InputAction, bool> key_map_conflicted;  // 506E6C
std::map<InputAction, PlatformKey> curr_key_map;

void Game_StartNewGameWhilePlaying(bool force_start) {
    if (confirmationState == CONFIRM_NEW_GAME || force_start) {
        engine->_messageQueue->clear();
        // pGUIWindow_CurrentMenu->Release();
        uGameState = GAME_STATE_NEWGAME_OUT_GAMEMENU;
        current_screen_type = SCREEN_GAME;
        engine->_statusBar->clearAll();
    } else {
        engine->_statusBar->setEvent(LSTR_START_NEW_GAME_PROMPT);
        pAudioPlayer->playUISound(SOUND_quest);
        confirmationState = CONFIRM_NEW_GAME;
    }
}

void Game_QuitGameWhilePlaying(bool force_quit) {
    if (confirmationState == CONFIRM_QUIT || force_quit) {
        engine->_messageQueue->clear();
        // pGUIWindow_CurrentMenu->Release();
        current_screen_type = SCREEN_GAME;
        pAudioPlayer->stopSounds();
        pAudioPlayer->playUISound(SOUND_WoodDoorClosing);
        uGameState = GAME_STATE_GAME_QUITTING_TO_MAIN_MENU;
        engine->_statusBar->clearAll();
    } else {
        engine->_statusBar->setEvent(LSTR_EXIT_GAME_PROMPT);
        pAudioPlayer->playUISound(SOUND_quest);
        confirmationState = CONFIRM_QUIT;
    }
}

void Game_OpenLoadGameDialog() {
    engine->_messageQueue->clear();
    pGUIWindow_CurrentMenu->Release();
    // TODO(captainurist): uncommenting delete pGUIWindow_CurrentMenu calls in this file leads to crashes, wtf?
    // delete pGUIWindow_CurrentMenu;
    pGUIWindow_CurrentMenu = nullptr;
    engine->_statusBar->clearEvent();
    // LoadUI_Load(1);
    current_screen_type = SCREEN_LOADGAME;
    pGUIWindow_CurrentMenu = new GUIWindow_Load(true);
    isLoadSlotClicked = false;
}

Menu::Menu() {
    mouse = EngineIocContainer::ResolveMouse();
}

Menu::~Menu() = default;

void Menu::EventLoop() {
    while (engine->_messageQueue->haveMessages()) {
        UIMessageType msg;
        int param, param2;
        engine->_messageQueue->popMessage(&msg, &param, &param2);

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
                --pSavegameList->saveListPosition;
                if (pSavegameList->saveListPosition < 0)
                    pSavegameList->saveListPosition = 0;
                new OnButtonClick2({215, 199}, {17, 17}, pBtnArrowUp);
                continue;

            case UIMSG_DownArrow:
                if (pSavegameList->saveListPosition + 7 < param) {
                    ++pSavegameList->saveListPosition;
                }
                new OnButtonClick2({215, 323}, {17, 17}, pBtnDownArrow);
                continue;

            case UIMSG_Cancel:
                new OnCancel({350, 302}, {106, 42}, pBtnCancel);
                continue;

            case UIMSG_SaveLoadBtn:
                new OnSaveLoad({241, 302}, {106, 42}, pBtnLoadSlot);
                continue;
            case UIMSG_SelectLoadSlot:
                if (pGUIWindow_CurrentMenu->keyboard_input_status == WINDOW_INPUT_IN_PROGRESS)
                    keyboardInputHandler->SetWindowInputStatus(WINDOW_INPUT_NONE);

                if (current_screen_type == SCREEN_SAVEGAME) {
                    if (pSavegameList->selectedSlot != pSavegameList->saveListPosition + param) {
                        pSavegameList->selectedSlot = pSavegameList->saveListPosition + param;
                    } else {
                        keyboardInputHandler->StartTextInput(TextInputType::Text, 19, pGUIWindow_CurrentMenu);
                        if (pSavegameList->pSavegameHeader[pSavegameList->selectedSlot].name != localization->GetString(LSTR_EMPTY_SAVESLOT)) {
                            keyboardInputHandler->SetTextInput(pSavegameList->pSavegameHeader[pSavegameList->selectedSlot].name);
                        }
                    }
                } else {
                    if (!isLoadSlotClicked || pSavegameList->selectedSlot != pSavegameList->saveListPosition + param) {
                        pSavegameList->selectedSlot = pSavegameList->saveListPosition + param;
                        isLoadSlotClicked = true;
                    } else {
                        engine->_messageQueue->addMessageCurrentFrame(UIMSG_LoadGame, 0, 0);
                    }
                }
                continue;
            case UIMSG_LoadGame:
                if (pSavegameList->pSavegameUsedSlots[pSavegameList->selectedSlot]) {
                    LoadGame(pSavegameList->selectedSlot);
                    uGameState = GAME_STATE_LOADING_GAME;
                }
                continue;
            case UIMSG_SaveGame:
                pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                if (pGUIWindow_CurrentMenu->keyboard_input_status == WINDOW_INPUT_IN_PROGRESS) {
                    keyboardInputHandler->SetWindowInputStatus(WINDOW_INPUT_NONE);
                    pSavegameList->pSavegameHeader[pSavegameList->selectedSlot].name = keyboardInputHandler->GetTextInput();
                }
                DoSavegame(pSavegameList->selectedSlot);
                continue;
            case UIMSG_Game_OpenSaveGameDialog: {
                if (engine->_currentLoadedMapId == MAP_ARENA) {
                    engine->_statusBar->setEvent(LSTR_NO_SAVING_IN_ARENA);
                    pAudioPlayer->playUISound(SOUND_error);
                } else {
                    pGUIWindow_CurrentMenu->Release();
                    // delete pGUIWindow_CurrentMenu;
                    engine->_statusBar->clearEvent();
                    current_screen_type = SCREEN_SAVEGAME;
                    pGUIWindow_CurrentMenu = new GUIWindow_Save();
                    // SaveUI_Load(current_screen_type = SCREEN_SAVEGAME);
                }
                continue;
            }
            case UIMSG_SaveLoadScroll: {
                // pskelton add for scroll click
                if (param < 7) {
                    // Too few saves to scroll yet
                    break;
                }
                int mx{}, my{};
                mouse->GetClickPos(&mx, &my);
                // 216 is offset down from top (216)
                my -= 216;
                // 107 is total height of bar
                float fmy = static_cast<float>(my) / 107.0f;
                int newlistpost = std::round((param - 7) * fmy);
                newlistpost = std::clamp(newlistpost, 0, (param - 7));
                pSavegameList->saveListPosition = newlistpost;
                pAudioPlayer->playUISound(SOUND_StartMainChoice02);
                continue;
            }
            case UIMSG_Game_OpenOptionsDialog:  // Open
            {
                engine->_messageQueue->clear();

                pGUIWindow_CurrentMenu->Release();
                // delete pGUIWindow_CurrentMenu;
                pGUIWindow_CurrentMenu = new GUIWindow_GameOptions();  // GameMenuUI_Options_Load();

                current_screen_type = SCREEN_OPTIONS;

                continue;
            }

            case UIMSG_OpenKeyMappingOptions:  // Open
            {
                engine->_messageQueue->clear();

                pGUIWindow_CurrentMenu->Release();
                // delete pGUIWindow_CurrentMenu;
                pGUIWindow_CurrentMenu = new GUIWindow_GameKeyBindings();  // GameMenuUI_OptionsKeymapping_Load();

                current_screen_type = SCREEN_KEYBOARD_OPTIONS;

                continue;
            }

            case UIMSG_ChangeKeyButton: {
                if (currently_selected_action_for_binding != Io::InputAction::Invalid) {
                    pAudioPlayer->playUISound(SOUND_error);
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
                pAudioPlayer->playUISound(SOUND_chimes);
                continue;
            }

            case UIMSG_SelectKeyPage1:
                KeyboardPageNum = 1;
                continue;
            case UIMSG_SelectKeyPage2:
                KeyboardPageNum = 2;
                continue;

            case UIMSG_OpenVideoOptions: {
                engine->_messageQueue->clear();

                pGUIWindow_CurrentMenu->Release();
                // delete pGUIWindow_CurrentMenu;
                pGUIWindow_CurrentMenu = new GUIWindow_GameVideoOptions();
                current_screen_type = SCREEN_VIDEO_OPTIONS;

                continue;
            }

            case UIMSG_ChangeGammaLevel: {
                int gammalevel = engine->config->graphics.Gamma.value();
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

                engine->config->graphics.Gamma.setValue(gammalevel);
                pAudioPlayer->playUISound(SOUND_ClickMovingSelector);

                if (gamma_preview_image) {
                    gamma_preview_image->Release();
                    gamma_preview_image = nullptr;
                }

                gamma_preview_image = GraphicsImage::Create(render->MakeViewportScreenshot(155, 117));
                continue;
            }
            case UIMSG_ToggleBloodsplats:
                engine->config->graphics.BloodSplats.toggle();
                continue;
            case UIMSG_ToggleColoredLights:
                engine->config->graphics.ColoredLights.toggle();
                continue;
            case UIMSG_ToggleTint:
                engine->config->graphics.Tinting.toggle();
                continue;

            case UIMSG_ChangeMusicVolume: {
                int new_level = engine->config->settings.MusicLevel.value();
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

                engine->config->settings.MusicLevel.setValue(new_level);
                pAudioPlayer->SetMusicVolume(engine->config->settings.MusicLevel.value());
                pAudioPlayer->playSound(SOUND_hurp, SOUND_MODE_MUSIC);
                continue;
            }

            case UIMSG_ChangeSoundVolume: {
                int new_level = engine->config->settings.SoundLevel.value();
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

                engine->config->settings.SoundLevel.setValue(new_level);

                pAudioPlayer->SetMasterVolume(engine->config->settings.SoundLevel.value());
                pAudioPlayer->playExclusiveSound(SOUND_church);
                continue;
            }
            case UIMSG_ToggleFlipOnExit:
                engine->config->settings.FlipOnExit.toggle();
                continue;
            case UIMSG_ToggleAlwaysRun:
                engine->config->settings.AlwaysRun.toggle();
                continue;
            case UIMSG_ToggleWalkSound:
                engine->config->settings.WalkSound.toggle();
                continue;
            case UIMSG_ToggleShowDamage:
                engine->config->settings.ShowHits.toggle();
                continue;
            case UIMSG_ChangeVoiceVolume: {
                int new_level = engine->config->settings.VoiceLevel.value();
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

                engine->config->settings.VoiceLevel.setValue(new_level);
                pAudioPlayer->SetVoiceVolume(engine->config->settings.VoiceLevel.value());
                if (engine->config->settings.VoiceLevel.value() > 0) {
                    pAudioPlayer->playSound(SOUND_hf445a, SOUND_MODE_SPEECH);
                }
                continue;
            }
            case UIMSG_SetTurnSpeed:
                if (param)
                    pParty->_viewYaw = param * pParty->_viewYaw / param;
                engine->config->settings.TurnSpeed.setValue(param);
                continue;

            case UIMSG_SetGraphicsMode:
                assert(false);  // Nomad: graphicsmode as it was now removed
                continue;

            case UIMSG_GameMenu_ReturnToGame:
                // pGUIWindow_CurrentMenu->Release();
                pEventTimer->setPaused(false);
                current_screen_type = SCREEN_GAME;
                continue;

            case UIMSG_Escape:
                if (pGameOverWindow) {
                    pGameOverWindow->Release();
                    pGameOverWindow = nullptr;
                    continue;
                }
                render->ClearZBuffer();
                confirmationState = CONFIRM_NONE;

                if (current_screen_type == SCREEN_MENU) {
                    pEventTimer->setPaused(false);
                    current_screen_type = SCREEN_GAME;
                } else if (current_screen_type == SCREEN_SAVEGAME ||
                           current_screen_type == SCREEN_LOADGAME) {
                    // crt_deconstruct_ptr_6A0118();

                    pGUIWindow_CurrentMenu->Release();
                    // delete pGUIWindow_CurrentMenu;
                    current_screen_type = SCREEN_MENU;
                    pGUIWindow_CurrentMenu = new GUIWindow_GameMenu();
                } else if (current_screen_type == SCREEN_OPTIONS) {
                    options_menu_skin.Release();
                    pGUIWindow_CurrentMenu->Release();
                    // delete pGUIWindow_CurrentMenu;
                    current_screen_type = SCREEN_MENU;
                    pGUIWindow_CurrentMenu = new GUIWindow_GameMenu();
                } else if (current_screen_type == SCREEN_VIDEO_OPTIONS) {
                    pGUIWindow_CurrentMenu->Release();
                    // delete pGUIWindow_CurrentMenu;
                    current_screen_type = SCREEN_MENU;
                    pGUIWindow_CurrentMenu = new GUIWindow_GameMenu();
                } else if (current_screen_type == SCREEN_KEYBOARD_OPTIONS) {
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
                        pAudioPlayer->playUISound(SOUND_error);
                        break; // deny to exit options until all key conflicts are solved
                    } else {
                        for (int i = 0; i < 5; i++) {
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
                    // delete pGUIWindow_CurrentMenu;
                    current_screen_type = SCREEN_MENU;
                    pGUIWindow_CurrentMenu = new GUIWindow_GameMenu();
                }
                continue;
            case UIMSG_QuickLoad:
                QuickLoadGame();
                break;
            default:
                break;
        }
    }
}

void Menu::MenuLoop() {
    pEventTimer->setPaused(true);
    current_screen_type = SCREEN_MENU;

    pGUIWindow_CurrentMenu = new GUIWindow_GameMenu();
    confirmationState = CONFIRM_NONE;

    if (gamma_preview_image) {
        gamma_preview_image->Release();
        gamma_preview_image = nullptr;
    }

    gamma_preview_image = GraphicsImage::Create(render->MakeViewportScreenshot(155, 117));

    pParty->resetCharacterEmotions();

    while (uGameState == GAME_STATE_PLAYING &&
           (current_screen_type == SCREEN_MENU ||
            current_screen_type == SCREEN_SAVEGAME ||
            current_screen_type == SCREEN_LOADGAME ||
            current_screen_type == SCREEN_OPTIONS ||
            current_screen_type == SCREEN_VIDEO_OPTIONS ||
            current_screen_type == SCREEN_KEYBOARD_OPTIONS)) {
        MessageLoopWithWait();

        GameUI_WritePointedObjectStatusString();
        render->BeginScene2D();
        engine->DrawGUI();
        GUI_UpdateWindows();
        engine->_statusBar->draw();
        render->Present();

        EventLoop();
    }

    pGUIWindow_CurrentMenu->Release();
    // delete pGUIWindow_CurrentMenu;
    pGUIWindow_CurrentMenu = nullptr;

    if (gamma_preview_image) {
        gamma_preview_image->Release();
        gamma_preview_image = nullptr;
    }
}
