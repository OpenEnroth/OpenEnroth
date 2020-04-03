#include "src/Application/GameWindowHandler.h"

#include "Arcomage/Arcomage.h"

#include "Engine/Engine.h"
#include "Engine/IocContainer.h"
#include "Engine/Party.h"
#include "Engine/Time.h"

#include "Engine/Graphics/IndoorCameraD3D.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"

#include "GUI/GUIWindow.h"

#include "IO/Keyboard.h"
#include "IO/Mouse.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/MediaPlayer.h"

#include "src/Application/IocContainer.h"


using EngineIoc = Engine_::IocContainer;
using ApplicationIoc = Application::IocContainer;
using Application::GameWindowHandler;


GameWindowHandler::GameWindowHandler() {
    this->mouse = EngineIoc::ResolveMouse();
}

void GameWindowHandler::OnScreenshot() {
    if (render) {
        render->SavePCXScreenshot();
    }
}

bool GameWindowHandler::OnChar(int c) {
    if (!pKeyActionMap->ProcessTextInput(c) && !viewparams->field_4C) {
        return GUI_HandleHotkey(c);
    }
    return 0;
}

void GameWindowHandler::OnMouseLeftClick(int x, int y) {
    if (pArcomageGame->bGameInProgress) {
        pArcomageGame->stru1.am_input_type = 7;
        pArcomageGame->check_exit = 0;
        pArcomageGame->force_redraw_1 = 1;
        ArcomageGame::OnMouseClick(0, true);
    } else {
        pMediaPlayer->StopMovie();

        mouse->SetMouseClick(x, y);

        if (GetCurrentMenuID() == MENU_CREATEPARTY) {
            UI_OnVkKeyDown(VK_SELECT);
        }

        if (engine) {
            engine->PickMouse(512.0, x, y, false, &vis_sprite_filter_3, &vis_door_filter);
        }

        mouse->UI_OnMouseLeftClick();
    }
}

void GameWindowHandler::OnMouseRightClick(int x, int y) {
    if (pArcomageGame->bGameInProgress) {
        pArcomageGame->stru1.am_input_type = 8;
        pArcomageGame->check_exit = 0;
        pArcomageGame->force_redraw_1 = 1;
        ArcomageGame::OnMouseClick(1, true);
    } else {
        pMediaPlayer->StopMovie();

        mouse->SetMouseClick(x, y);

        if (engine) {
            engine->PickMouse(pIndoorCameraD3D->GetPickDepth(), x, y, 0, &vis_sprite_filter_2, &vis_door_filter);
        }

        UI_OnMouseRightClick(x, y);
    }
}

void GameWindowHandler::OnMouseLeftUp() {
    if (pArcomageGame->bGameInProgress) {
        pArcomageGame->stru1.am_input_type = 3;
        ArcomageGame::OnMouseClick(0, 0);
    } else {
        back_to_game();
    }
}

void GameWindowHandler::OnMouseRightUp() {
    if (pArcomageGame->bGameInProgress) {
        pArcomageGame->stru1.am_input_type = 4;
        ArcomageGame::OnMouseClick(1, false);
    } else {
        back_to_game();
    }
}

void GameWindowHandler::OnMouseLeftDoubleClick(int x, int y) {
    if (pArcomageGame->bGameInProgress) {
        pArcomageGame->stru1.am_input_type = 7;
    } else {
        OnMouseLeftClick(x, y);
    }
}

void GameWindowHandler::OnMouseRightDoubleClick(int x, int y) {
    if (pArcomageGame->bGameInProgress) {
        pArcomageGame->stru1.am_input_type = 8;
    } else {
        OnMouseRightClick(x, y);
    }
}

void GameWindowHandler::OnMouseMove(int x, int y, bool left_button, bool right_button) {
    if (pArcomageGame->bGameInProgress) {
        ArcomageGame::OnMouseMove(x, y);
        ArcomageGame::OnMouseClick(0, left_button);
        ArcomageGame::OnMouseClick(1, right_button);
    } else {
        if (mouse) {
            mouse->SetMouseClick(x, y);
        }
    }
}

extern bool _507B98_ctrl_pressed;

void GameWindowHandler::OnVkDown(int vk, int vk_to_char) {
    if (uGameMenuUI_CurentlySelectedKeyIdx != -1) {
        pKeyActionMap->ProcessTextInput(vk);
    } else if (pArcomageGame->bGameInProgress) {
        pArcomageGame->stru1.am_input_type = 1;

        set_stru1_field_8_InArcomage(vk_to_char);
        if (vk == VK_ESCAPE) {
            pArcomageGame->stru1.am_input_type = 10;
        } else if (pArcomageGame->check_exit) {
           pArcomageGame->check_exit = 0;
           pArcomageGame->force_redraw_1 = 1;
        }

        if (vk == VK_F3) {
            OnScreenshot();
        } else if (vk == VK_F4 && !pMovie_Track) {
            OnToggleFullscreen();
            pArcomageGame->stru1.am_input_type = 9;
        }
    } else {
        pMediaPlayer->StopMovie();
        if (vk == VK_RETURN) {
            if (!viewparams->field_4C) UI_OnVkKeyDown(vk);
        } else if (vk == VK_CONTROL) {
            _507B98_ctrl_pressed = true;
        } else if (vk == VK_ESCAPE) {
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, window_SpeakInHouse != 0, 0);
        } else if (vk == VK_F4 && !pMovie_Track) {
            OnToggleFullscreen();
        } else if (vk == VK_NUMPAD0) {
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_OpenDebugMenu, window_SpeakInHouse != 0, 0);
        } else if (vk >= VK_LEFT && vk <= VK_DOWN) {
            if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME &&
                current_screen_type != CURRENT_SCREEN::SCREEN_MODAL_WINDOW) {
                if (!viewparams->field_4C) {
                    UI_OnVkKeyDown(vk);
                }
            }
        }
    }
}

void GameWindowHandler::OnFocus() {
    __debugbreak();
}

void GameWindowHandler::OnFocusLost() {
    __debugbreak();
}

void GameWindowHandler::OnPaint() {
    if (pArcomageGame->bGameInProgress) {
        pArcomageGame->force_redraw_1 = 1;
    }
    if (render && render->AreRenderSurfacesOk()) {
        render->Present();
    }
}

void GameWindowHandler::OnActivated() {
    if (dword_6BE364_game_settings_1 & GAME_SETTINGS_APP_INACTIVE) {
        // dword_4E98BC_bApplicationActive = 1;
        //        Resume video playback
        //          pMediaPlayer->
        //          if (pMovie_Track)
        //            pMediaPlayer->bPlaying_Movie = true;

        dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_APP_INACTIVE;

        if (pArcomageGame->bGameInProgress) {
            pArcomageGame->force_redraw_1 = 1;
        } else {
            if (dword_6BE364_game_settings_1 & GAME_SETTINGS_0200_EVENT_TIMER)
                dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_0200_EVENT_TIMER;
            else
                pEventTimer->Resume();
            if (dword_6BE364_game_settings_1 & GAME_SETTINGS_0400_MISC_TIMER)
                dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_0400_MISC_TIMER;
            else
                pMiscTimer->Resume();

            viewparams->bRedrawGameUI = true;
            if (pMovie_Track) {  // pVideoPlayer->pSmackerMovie )
                render->RestoreFrontBuffer();
                render->RestoreBackBuffer();
                // BackToHouseMenu();
            }
        }

        if (!bGameoverLoop && !pMovie_Track) {  // continue an audio track
            pAudioPlayer->MusicResume();
        }
    }
}

void GameWindowHandler::OnDeactivated() {
    if (!(dword_6BE364_game_settings_1 & GAME_SETTINGS_APP_INACTIVE)) {
        // dword_4E98BC_bApplicationActive = 0;

        dword_6BE364_game_settings_1 |= GAME_SETTINGS_APP_INACTIVE;
        if (pEventTimer->bPaused)
            dword_6BE364_game_settings_1 |= GAME_SETTINGS_0200_EVENT_TIMER;
        else
            pEventTimer->Pause();
        if (pMiscTimer->bPaused)
            dword_6BE364_game_settings_1 |= GAME_SETTINGS_0400_MISC_TIMER;
        else
            pMiscTimer->Pause();

        if (pAudioPlayer) {
            pAudioPlayer->StopChannels(-1, -1);
            pAudioPlayer->MusicPause();
        }
    }
}

void GameWindowHandler::OnToggleFullscreen() {
    __debugbreak();
}
