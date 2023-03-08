#include "GameWindowHandler.h"

#include <algorithm>
#include <string>
#include <vector>

#include "Arcomage/Arcomage.h"

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/Nuklear.h"
#include "Engine/Graphics/NuklearEventHandler.h"
#include "Engine/IocContainer.h"
#include "Engine/Party.h"
#include "Engine/Time.h"

#include "GUI/GUIWindow.h"

#include "Io/InputAction.h"
#include "Io/KeyboardInputHandler.h"
#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/MediaPlayer.h"

#include "IocContainer.h"


using EngineIoc = Engine_::IocContainer;
using ApplicationIoc = Application::IocContainer;
using Application::GameWindowHandler;
using Io::InputAction;

static char PlatformKeyToChar(PlatformKey key, PlatformModifiers mods) {
    if (key >= PlatformKey::Digit0 && key <= PlatformKey::Digit9) {
        return std::to_underlying(key) - std::to_underlying(PlatformKey::Digit0) + '0';
    } else if (key >= PlatformKey::A && key <= PlatformKey::Z) {
        if (mods & MOD_SHIFT) {
            return std::to_underlying(key) - std::to_underlying(PlatformKey::A) + 'A';
        } else {
            return std::to_underlying(key) - std::to_underlying(PlatformKey::A) + 'a';
        }
    }

    return 0;
}


GameWindowHandler::GameWindowHandler() : PlatformEventFilter(EVENTS_ALL) {
    this->mouse = EngineIoc::ResolveMouse();
    this->keyboardController_ = std::make_unique<GameKeyboardController>();
}

GameWindowHandler::~GameWindowHandler() {}

std::tuple<int, Pointi, Sizei> GameWindowHandler::GetWindowConfigPosition(const GameConfig *config) {
    std::vector<Recti> displays = platform->displayGeometries();

    Pointi pos = {config->window.PositionX.Get(), config->window.PositionY.Get()};
    Sizei size = {config->window.Width.Get(), config->window.Height.Get()};
    PlatformWindowMode mode = config->window.Mode.Get();

    Recti displayRect;
    int display = config->window.Display.Get();
    if (display > 0 && display < displays.size()) {
        displayRect = displays[display];
    } else if (!displays.empty()) {
        displayRect = displays[0];
    }

    if (mode == WINDOW_MODE_FULLSCREEN || mode == WINDOW_MODE_FULLSCREEN_BORDERLESS) {
        pos = displayRect.TopLeft();
    } else if (Recti(Pointi(), displayRect.Size()).Contains(pos)) {
        pos += displayRect.TopLeft();
    } else if (displayRect != Recti()) {
        pos = displayRect.Center() - Pointi(size.w, size.h) / 2;
    } else {
        pos = Pointi(0, 0); // DisplayGeometries() failed so we just give up.
    }

    return std::make_tuple(display, pos, size);
}

std::tuple<int, Pointi, Sizei> GameWindowHandler::GetWindowRelativePosition(Pointi *position) {
    std::vector<Recti> displays = platform->displayGeometries();

    Sizei size = window->size();
    Pointi pos;
    if (position != nullptr)
        pos = *position;
    else
        pos = window->position();

    // Fallback is centered on display 0.
    Pointi relativePos = Pointi(-1, -1);
    int display = 0;

    for (size_t i = 0; i < displays.size(); i++) {
        if (displays[i].Contains(pos)) {
            display = i;
            relativePos = pos - displays[i].TopLeft();
            break;
        }
    }

    return std::make_tuple(display, relativePos, size);
}

void GameWindowHandler::UpdateWindowFromConfig(const GameConfig *config) {
    assert(config);

    Sizei size = {config->window.Width.Get(), config->window.Height.Get()};
    Pointi pos = std::get<1>(GetWindowConfigPosition(config));
    PlatformWindowMode mode = config->window.Mode.Get();

    window->setPosition(pos);
    window->resize(size);
    window->setTitle(config->window.Title.Get());
    window->setGrabsMouse(config->window.MouseGrab.Get());
    window->setWindowMode(mode);
    window->setResizable(config->window.Resizable.Get());
    window->setVisible(true);
}

void GameWindowHandler::UpdateConfigFromWindow(GameConfig *config) {
    assert(config);

    auto [display, relativePos, wsize] = GetWindowRelativePosition();

    // Skip updating window position in fullscreen where window always forcefully moved to {0,0} position.
    PlatformWindowMode mode = window->windowMode();
    if (mode == WINDOW_MODE_WINDOWED || mode == WINDOW_MODE_BORDERLESS) {
        config->window.PositionX.Set(relativePos.x);
        config->window.PositionY.Set(relativePos.y);
        config->window.Display.Set(display);
    }
    // Skip updating window dimensions in borderless fullscreen as window always resized to monitor's resolution.
    if (mode != WINDOW_MODE_FULLSCREEN_BORDERLESS) {
        config->window.Width.Set(wsize.w);
        config->window.Height.Set(wsize.h);
    }
    config->window.Mode.Set(mode);
    config->window.MouseGrab.Set(window->grabsMouse());
    config->window.Resizable.Set(window->isResizable());
}

void GameWindowHandler::OnScreenshot() {
    if (render) {
        render->SavePCXScreenshot();
    }
}

bool GameWindowHandler::OnChar(PlatformKey key, int c) {
    bool textInputHandled = false;

    if (!keyboardInputHandler)
        return false;

    // backspace, enter, esc (text input), controls binding
    textInputHandled |= keyboardInputHandler->ProcessTextInput(key, c);

    // regular text input
    if (c != -1) {
        textInputHandled |= keyboardInputHandler->ProcessTextInput(PlatformKey::Char, c);
    }

    if (!textInputHandled && !viewparams->field_4C) {
        return GUI_HandleHotkey(key);  // try other hotkeys
    }
    return false;
}

Pointi GameWindowHandler::MapToRender(Pointi position) {
    Sizef renDims = {(float)render->GetRenderDimensions().w, (float)render->GetRenderDimensions().h};
    Sizef prDims = {(float)render->GetPresentDimensions().w, (float)render->GetPresentDimensions().h};
    Pointi result = position;

    if (renDims != prDims) {
        Sizef ratioCorections = {prDims.w / renDims.w, prDims.h / renDims.h};
        float ratioCorrection = std::min(ratioCorections.w, ratioCorections.h);

        float w = renDims.w * ratioCorrection;
        float h = renDims.h * ratioCorrection;

        result.x = (float)position.x / ratioCorrection - ((float)prDims.w / 2 - w / 2) / ratioCorrection;
        result.y = (float)position.y / ratioCorrection - ((float)prDims.h / 2 - h / 2) / ratioCorrection;
    }

    return result;
}

void GameWindowHandler::OnMouseLeftClick(Pointi position) {
    if (pArcomageGame->bGameInProgress) {
        ArcomageGame::OnMouseClick(0, true);
    } else {
        pMediaPlayer->StopMovie();

        mouse->SetMouseClick(position.x, position.y);

        if (GetCurrentMenuID() == MENU_CREATEPARTY) {
            UI_OnKeyDown(PlatformKey::Select);
        }

        if (engine) {
            engine->PickMouse(engine->config->gameplay.MouseInteractionDepth.Get(), position.x, position.y, false,
                              &vis_sprite_filter_3, &vis_door_filter);
        }

        mouse->UI_OnMouseLeftClick();
    }
}

void GameWindowHandler::OnMouseRightClick(Pointi position) {
    if (pArcomageGame->bGameInProgress) {
        ArcomageGame::OnMouseClick(1, true);
    } else {
        pMediaPlayer->StopMovie();

        mouse->SetMouseClick(position.x, position.y);

        if (engine) {
            engine->PickMouse(pCamera3D->GetMouseInfoDepth(), position.x, position.y, 0, &vis_sprite_filter_2, &vis_door_filter);
        }

        UI_OnMouseRightClick(position.x, position.y);
    }
}

void GameWindowHandler::OnMouseLeftUp() {
    if (pArcomageGame->bGameInProgress) {
        ArcomageGame::OnMouseClick(0, 0);
    } else {
        back_to_game();
    }
}

void GameWindowHandler::OnMouseRightUp() {
    if (pArcomageGame->bGameInProgress) {
        ArcomageGame::OnMouseClick(1, false);
    } else {
        back_to_game();
    }
}

void GameWindowHandler::OnMouseLeftDoubleClick(Pointi position) {
    if (pArcomageGame->bGameInProgress) {
        pArcomageGame->OnMouseClick(0, true);
    } else {
        OnMouseLeftClick(position);
    }
}

void GameWindowHandler::OnMouseRightDoubleClick(Pointi position) {
    if (pArcomageGame->bGameInProgress) {
        pArcomageGame->OnMouseClick(1, true);
    } else {
        OnMouseRightClick(position);
    }
}

void GameWindowHandler::OnMouseMove(Pointi position, bool left_button, bool right_button) {
    if (pArcomageGame->bGameInProgress) {
        ArcomageGame::OnMouseMove(position.x, position.y);
        ArcomageGame::OnMouseClick(0, left_button);
        ArcomageGame::OnMouseClick(1, right_button);
    } else {
        if (mouse) {
            mouse->SetMouseClick(position.x, position.y);
        }
    }
}


extern InputAction currently_selected_action_for_binding;

void GameWindowHandler::OnKey(PlatformKey key) {
    if (!keyboardInputHandler || !keyboardActionMapping)
        return;

    // TODO: many of hardcoded keys below should be moved out of there and made configurable
    if (keyboardActionMapping->IsKeyMatchAction(InputAction::ToggleMouseGrab, key)) {
        OnMouseGrabToggle();
        return;
    }  else if (keyboardActionMapping->IsKeyMatchAction(InputAction::Screenshot, key)) {
        OnScreenshot();
        return;
    } else if (keyboardActionMapping->IsKeyMatchAction(InputAction::ToggleBorderless, key)) {
        OnToggleBorderless();
        return;
    } else if (keyboardActionMapping->IsKeyMatchAction(InputAction::ToggleResizable, key)) {
        OnToggleResizable();
        return;
    } else if (keyboardActionMapping->IsKeyMatchAction(InputAction::CycleFilter, key)) {
        OnCycleFilter();
        return;
    }

    if (currently_selected_action_for_binding != InputAction::Invalid) {
        // we're setting a key binding in options
        keyboardInputHandler->ProcessTextInput(key, -1);
    } else if (pArcomageGame->bGameInProgress) {
        // TODO(pskelton): how should this be handled?
        if (keyboardActionMapping->IsKeyMatchAction(InputAction::ToggleFullscreen, key) && !pMovie_Track) {
            OnToggleFullscreen();
        }
        pArcomageGame->onKeyPress(key);
    } else {
        pMediaPlayer->StopMovie();
        if (keyboardActionMapping->IsKeyMatchAction(InputAction::Return, key)) {
            if (!viewparams->field_4C)
                UI_OnKeyDown(key);
        } else if (keyboardActionMapping->IsKeyMatchAction(InputAction::Escape, key)) {
            pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, window_SpeakInHouse != 0, 0);
        } else if (keyboardActionMapping->IsKeyMatchAction(InputAction::ToggleFullscreen, key) && !pMovie_Track) {
            OnToggleFullscreen();
        } else if (keyboardActionMapping->IsKeyMatchAction(InputAction::Console, key)) {
            pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_OpenDebugMenu, window_SpeakInHouse != 0, 0);
        } else if (keyboardActionMapping->IsKeyMatchAction(InputAction::ReloadShaders, key) && current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
            pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_DebugReloadShader, window_SpeakInHouse != 0, 0);
        } else if (keyboardActionMapping->IsKeyMatchAction(InputAction::DialogLeft, key) || keyboardActionMapping->IsKeyMatchAction(InputAction::DialogRight, key)
            || keyboardActionMapping->IsKeyMatchAction(InputAction::DialogUp, key) || keyboardActionMapping->IsKeyMatchAction(InputAction::DialogDown, key)
            || keyboardActionMapping->IsKeyMatchAction(InputAction::DialogSelect, key)) {
            if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME &&
                current_screen_type != CURRENT_SCREEN::SCREEN_GAMEOVER_WINDOW) {
                if (!viewparams->field_4C) {
                    UI_OnKeyDown(key);
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

        render->Reinitialize();

        dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_APP_INACTIVE;

        if (!pArcomageGame->bGameInProgress) {
            if (dword_6BE364_game_settings_1 & GAME_SETTINGS_0200_EVENT_TIMER)
                dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_0200_EVENT_TIMER;
            else
                pEventTimer->Resume();
            if (dword_6BE364_game_settings_1 & GAME_SETTINGS_0400_MISC_TIMER)
                dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_0400_MISC_TIMER;
            else
                pMiscTimer->Resume();

            if (pMovie_Track) {  // pVideoPlayer->pSmackerMovie )
                render->RestoreFrontBuffer();
                render->RestoreBackBuffer();
                // BackToHouseMenu();
            }
        }

        pAudioPlayer->ResumeSounds();
        if (!bGameoverLoop && !pMovie_Track) {  // continue an audio track
            pAudioPlayer->MusicResume();
        }
    }
}

void GameWindowHandler::OnDeactivated() {
    if (!(dword_6BE364_game_settings_1 & GAME_SETTINGS_APP_INACTIVE)) {
        // dword_4E98BC_bApplicationActive = 0;

        dword_6BE364_game_settings_1 |= GAME_SETTINGS_APP_INACTIVE;
        if (pEventTimer != nullptr) {
            if (pEventTimer->bPaused)
                dword_6BE364_game_settings_1 |= GAME_SETTINGS_0200_EVENT_TIMER;
            else
                pEventTimer->Pause();
        }

        if (pMiscTimer != nullptr) {
            if (pMiscTimer->bPaused)
                dword_6BE364_game_settings_1 |= GAME_SETTINGS_0400_MISC_TIMER;
            else
                pMiscTimer->Pause();
        }

        if (pAudioPlayer != nullptr) {
            pAudioPlayer->PauseSounds(2);
            pAudioPlayer->MusicPause();
        }
    }
}

void GameWindowHandler::OnToggleBorderless() {
    PlatformWindowMode mode = window->windowMode();
    switch (mode) {
        case WINDOW_MODE_FULLSCREEN:
            mode = WINDOW_MODE_FULLSCREEN_BORDERLESS;
            break;
        case WINDOW_MODE_FULLSCREEN_BORDERLESS:
            mode = WINDOW_MODE_FULLSCREEN;
            break;
        case WINDOW_MODE_WINDOWED:
            mode = WINDOW_MODE_BORDERLESS;
            break;
        case WINDOW_MODE_BORDERLESS:
            mode = WINDOW_MODE_WINDOWED;
            break;
        default:
            assert(false); //should never get there.
            break;
    }

    window->setWindowMode(mode);
}

void GameWindowHandler::OnToggleFullscreen() {
    PlatformWindowMode mode = window->windowMode();
    switch (mode) {
        case WINDOW_MODE_FULLSCREEN:
            mode = WINDOW_MODE_WINDOWED;
            break;
        case WINDOW_MODE_FULLSCREEN_BORDERLESS:
            mode = WINDOW_MODE_BORDERLESS;
            break;
        case WINDOW_MODE_WINDOWED:
            mode = WINDOW_MODE_FULLSCREEN;
            break;
        case WINDOW_MODE_BORDERLESS:
            mode = WINDOW_MODE_FULLSCREEN_BORDERLESS;
            break;
        default:
            assert(false); //should never get there.
            break;
    }

    window->setWindowMode(mode);
    engine->config->window.Mode.Set(mode);
    if (mode == WINDOW_MODE_WINDOWED || mode == WINDOW_MODE_BORDERLESS) {
        window->resize({engine->config->window.Width.Get(), engine->config->window.Height.Get()});
        window->setPosition(std::get<1>(GetWindowConfigPosition(engine->config.get())));
    }
    render->Reinitialize();
}

void GameWindowHandler::OnToggleResizable() {
    window->setResizable(engine->config->window.Resizable.Toggle());
}

void GameWindowHandler::OnCycleFilter() {
    engine->config->graphics.RenderFilter.CycleIncrement();
    render->Reinitialize();
}

void GameWindowHandler::OnMouseGrabToggle() {
    window->setGrabsMouse(engine->config->window.MouseGrab.Toggle());
}

void GameWindowHandler::handleKeyPress(PlatformKey key, PlatformModifiers mods, bool isAutoRepeat) {
    keyboardController_->ProcessKeyPressEvent(key);

    if (isAutoRepeat)
        return;

    char c = PlatformKeyToChar(key, mods);

    OnChar(key, -1);
    OnKey(key);

    if (c != 0)
        OnChar(key, c);
}

void GameWindowHandler::handleKeyRelease(PlatformKey key) {
    keyboardController_->ProcessKeyReleaseEvent(key);
}

bool GameWindowHandler::keyPressEvent(const PlatformKeyEvent *event) {
    handleKeyPress(event->key, event->mods, event->isAutoRepeat);
    return false;
}

bool GameWindowHandler::keyReleaseEvent(const PlatformKeyEvent *event) {
    handleKeyRelease(event->key);
    return false;
}

bool GameWindowHandler::mouseMoveEvent(const PlatformMouseEvent *event) {
    OnMouseMove(MapToRender(event->pos), event->buttons & BUTTON_LEFT, event->buttons & BUTTON_RIGHT);
    return false;
}

bool GameWindowHandler::mousePressEvent(const PlatformMouseEvent *event) {
    Pointi position = MapToRender(event->pos);
    if (event->button == BUTTON_LEFT) {
        if (event->isDoubleClick) {
            OnMouseLeftDoubleClick(position);
        } else {
            OnMouseLeftClick(position);
        }
    } else if (event->button == BUTTON_RIGHT) {
        if (event->isDoubleClick) {
            OnMouseRightDoubleClick(position);
        } else {
            OnMouseRightClick(position);
        }
    }
    return false;
}

bool GameWindowHandler::mouseReleaseEvent(const PlatformMouseEvent *event) {
    if (event->button == BUTTON_LEFT) {
        OnMouseLeftUp();
    } else if (event->button == BUTTON_RIGHT) {
        OnMouseRightUp();
    }
    return false;
}

bool GameWindowHandler::wheelEvent(const PlatformWheelEvent *) { return false; }

bool GameWindowHandler::moveEvent(const PlatformMoveEvent *event) {
    /* Remember window position after move. Move position event is also triggered on toggling fullscreen. And we should save current window position prior to entering fullscreen.
     * As entering fullscreen will forcefully move window to {0,0} position on current display. And we want to restore position prior to entering fullscreen and not {0,0} or startup one. */
    PlatformWindowMode mode = window->windowMode();
    if (mode == WINDOW_MODE_WINDOWED || mode == WINDOW_MODE_BORDERLESS) {
        Pointi pos = event->pos;
        auto [display, relativePos, wsize] = GetWindowRelativePosition(&pos);

        engine->config->window.PositionX.Set(relativePos.x);
        engine->config->window.PositionY.Set(relativePos.y);
        engine->config->window.Display.Set(display);
    }
    return false;
}

bool GameWindowHandler::resizeEvent(const PlatformResizeEvent *event) {
    render->Reinitialize();
    return false;
}

bool GameWindowHandler::activationEvent(const PlatformWindowEvent *event) {
    if (event->type == EVENT_WINDOW_ACTIVATE) {
        OnActivated();
    } else if (event->type == EVENT_WINDOW_DEACTIVATE) {
        OnDeactivated();
    }
    return false;
}

bool GameWindowHandler::closeEvent(const PlatformWindowEvent *event) {
    UpdateConfigFromWindow(engine->config.get());
    engine->config->SaveConfiguration();
    Engine_DeinitializeAndTerminate(0);
    return false;
}

bool GameWindowHandler::gamepadConnectionEvent(const PlatformGamepadEvent *event) {
    logger->Info("Gamepad {}, model='{}', serial='{}'",
                 event->type == EVENT_GAMEPAD_CONNECTED ? "connected" : "disconnected",
                 event->gamepad->model(), event->gamepad->serial());
    return false;
}

bool GameWindowHandler::gamepadKeyPressEvent(const PlatformGamepadKeyEvent *event) {
    handleKeyPress(event->key, 0, false);
    return false;
}

bool GameWindowHandler::gamepadKeyReleaseEvent(const PlatformGamepadKeyEvent *event) {
    handleKeyRelease(event->key);
    return false;
}

bool GameWindowHandler::gamepadAxisEvent(const PlatformGamepadAxisEvent *event) {
    PlatformKey key = event->axis;
    float value = event->value;

    // TODO(captainurist): this is temporary, we need separate axis enum and proper axis handling
    if (value < 0) {
        if (key == PlatformKey::Gamepad_LeftStick_Right) {
            key = PlatformKey::Gamepad_LeftStick_Left;
        } else if (key == PlatformKey::Gamepad_RightStick_Right) {
            key = PlatformKey::Gamepad_RightStick_Left;
        } else if (key == PlatformKey::Gamepad_LeftStick_Down) {
            key = PlatformKey::Gamepad_LeftStick_Up;
        } else if (key == PlatformKey::Gamepad_RightStick_Down) {
            key = PlatformKey::Gamepad_RightStick_Up;
        }

        value = -value;
    }

    // TODO: deadzone should be configurable and default should be lowered once we implement proper axis event processing.
    if (value >= 0.5) {
        handleKeyPress(key, 0, false);
    } else {
        handleKeyRelease(key);
    }

    return false;
}
