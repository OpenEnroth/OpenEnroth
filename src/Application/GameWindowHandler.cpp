#include "GameWindowHandler.h"

#include <algorithm>
#include <vector>
#include <tuple>
#include <string>

#include "Arcomage/Arcomage.h"

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Components/Control/EngineControlComponent.h"
#include "Engine/Components/Control/EngineController.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/EngineIocContainer.h"
#include "Engine/Time/Timer.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIMessageQueue.h"

#include "Io/InputAction.h"
#include "Io/KeyboardInputHandler.h"
#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/MediaPlayer.h"

#include "Library/Image/PCX.h"
#include "Library/Logger/Logger.h"
#include "Library/Platform/Application/PlatformApplication.h"
#include "Library/Platform/Interface/PlatformGamepad.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/DataPath.h"

using Io::InputAction;

static char PlatformKeyToChar(PlatformKey key, PlatformModifiers mods) {
    if (key >= PlatformKey::KEY_DIGIT_0 && key <= PlatformKey::KEY_DIGIT_9) {
        return std::to_underlying(key) - std::to_underlying(PlatformKey::KEY_DIGIT_0) + '0';
    } else if (key >= PlatformKey::KEY_NUMPAD_0 && key <= PlatformKey::KEY_NUMPAD_9 && (mods & MOD_NUM)) {
        return std::to_underlying(key) - std::to_underlying(PlatformKey::KEY_NUMPAD_0) + '0';
    } else if (key >= PlatformKey::KEY_A && key <= PlatformKey::KEY_Z) {
        if (mods & MOD_SHIFT) {
            return std::to_underlying(key) - std::to_underlying(PlatformKey::KEY_A) + 'A';
        } else {
            return std::to_underlying(key) - std::to_underlying(PlatformKey::KEY_A) + 'a';
        }
    }

    return 0;
}


GameWindowHandler::GameWindowHandler() : PlatformEventFilter(EVENTS_ALL) {
    this->mouse = EngineIocContainer::ResolveMouse();
}

GameWindowHandler::~GameWindowHandler() {}

std::tuple<int, Pointi, Sizei> GameWindowHandler::GetWindowConfigPosition(const GameConfig *config) {
    std::vector<Recti> displays = platform->displayGeometries();

    Pointi pos = {config->window.PositionX.value(), config->window.PositionY.value()};
    Sizei size = {config->window.Width.value(), config->window.Height.value()};
    PlatformWindowMode mode = config->window.Mode.value();

    Recti displayRect;
    int display = config->window.Display.value();
    if (display > 0 && display < displays.size()) {
        displayRect = displays[display];
    } else if (!displays.empty()) {
        displayRect = displays[0];
    }

    if (mode == WINDOW_MODE_FULLSCREEN || mode == WINDOW_MODE_FULLSCREEN_BORDERLESS) {
        pos = displayRect.topLeft();
    } else if (Recti(Pointi(), displayRect.size()).contains(pos)) {
        pos += displayRect.topLeft();
    } else if (displayRect != Recti()) {
        pos = displayRect.center() - Pointi(size.w, size.h) / 2;
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
        if (displays[i].contains(pos)) {
            display = i;
            relativePos = pos - displays[i].topLeft();
            break;
        }
    }

    return std::make_tuple(display, relativePos, size);
}

void GameWindowHandler::UpdateWindowFromConfig(const GameConfig *config) {
    assert(config);

    Sizei size = {config->window.Width.value(), config->window.Height.value()};
    Pointi pos = std::get<1>(GetWindowConfigPosition(config));
    PlatformWindowMode mode = config->window.Mode.value();

    window->setPosition(pos);
    window->resize(size);
    window->setTitle(config->window.Title.value());
    window->setGrabsMouse(config->window.MouseGrab.value());
    window->setWindowMode(mode);
    window->setResizable(config->window.Resizable.value());
    window->setVisible(true);
}

void GameWindowHandler::UpdateConfigFromWindow(GameConfig *config) {
    assert(config);

    auto [display, relativePos, wsize] = GetWindowRelativePosition();

    // Skip updating window position in fullscreen where window always forcefully moved to {0,0} position.
    PlatformWindowMode mode = window->windowMode();
    if (mode == WINDOW_MODE_WINDOWED || mode == WINDOW_MODE_BORDERLESS) {
        config->window.PositionX.setValue(relativePos.x);
        config->window.PositionY.setValue(relativePos.y);
        config->window.Display.setValue(display);
    }
    // Skip updating window dimensions in borderless fullscreen as window always resized to monitor's resolution.
    if (mode != WINDOW_MODE_FULLSCREEN_BORDERLESS) {
        config->window.Width.setValue(wsize.w);
        config->window.Height.setValue(wsize.h);
    }
    config->window.Mode.setValue(mode);
    config->window.MouseGrab.setValue(window->grabsMouse());
    config->window.Resizable.setValue(window->isResizable());
}

void GameWindowHandler::OnScreenshot() {
    if (render) {
        // TODO(pskelton): add "Screenshots" folder?
        engine->config->settings.ScreenshotNumber.increment();
        std::string path = fmt::format("screenshot_{:05}.pcx", engine->config->settings.ScreenshotNumber.value());

        FileOutputStream(makeDataPath(path)).write(pcx::encode(render->MakeFullScreenshot()).string_view());
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
        textInputHandled |= keyboardInputHandler->ProcessTextInput(PlatformKey::KEY_CHAR, c);
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
            UI_OnKeyDown(PlatformKey::KEY_SELECT);
        }

        if (engine) {
            engine->PickMouse(engine->config->gameplay.MouseInteractionDepth.value(), position.x, position.y,
                              &vis_decoration_noevent_filter, &vis_door_filter);
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
            engine->PickMouse(pCamera3D->GetMouseInfoDepth(), position.x, position.y, &vis_allsprites_filter, &vis_door_filter);
        }

        UI_OnMouseRightClick(position.x, position.y);
    }
}

void GameWindowHandler::OnMouseLeftUp() {
    if (pArcomageGame->bGameInProgress) {
        ArcomageGame::OnMouseClick(0, 0);
    } else if (!isHoldingMouseRightButton()) {
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

    if (currently_selected_action_for_binding != Io::InputAction::Invalid) {
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
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, window_SpeakInHouse != 0, 0);
        } else if (keyboardActionMapping->IsKeyMatchAction(InputAction::ToggleFullscreen, key) && !pMovie_Track) {
            OnToggleFullscreen();
        } else if (keyboardActionMapping->IsKeyMatchAction(InputAction::Console, key)) {
            engine->toggleOverlays();
        } else if (keyboardActionMapping->IsKeyMatchAction(InputAction::ReloadShaders, key) && current_screen_type == SCREEN_GAME) {
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_DebugReloadShader, window_SpeakInHouse != 0, 0);
        } else if (keyboardActionMapping->IsKeyMatchAction(InputAction::QuickSave, key) && current_screen_type == SCREEN_GAME) {
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_QuickSave, window_SpeakInHouse != 0, 0);
        } else if (keyboardActionMapping->IsKeyMatchAction(InputAction::QuickLoad, key)) {
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_QuickLoad, window_SpeakInHouse != 0, 0);
        } else if (keyboardActionMapping->IsKeyMatchAction(InputAction::DialogLeft, key) || keyboardActionMapping->IsKeyMatchAction(InputAction::DialogRight, key)
            || keyboardActionMapping->IsKeyMatchAction(InputAction::DialogUp, key) || keyboardActionMapping->IsKeyMatchAction(InputAction::DialogDown, key)
            || keyboardActionMapping->IsKeyMatchAction(InputAction::DialogSelect, key)) {
            if (current_screen_type != SCREEN_GAME &&
                current_screen_type != SCREEN_GAMEOVER_WINDOW) {
                if (!viewparams->field_4C) {
                    UI_OnKeyDown(key);
                }
            }
        }
    }
}

void GameWindowHandler::OnFocus() {
    assert(false);
}

void GameWindowHandler::OnFocusLost() {
    assert(false);
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
                pEventTimer->setPaused(false);
            if (dword_6BE364_game_settings_1 & GAME_SETTINGS_0400_MISC_TIMER)
                dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_0400_MISC_TIMER;
            else
                pMiscTimer->setPaused(false);

            if (pMovie_Track) {  // pVideoPlayer->pSmackerMovie )
                render->RestoreFrontBuffer();
                render->RestoreBackBuffer();
                // BackToHouseMenu();
            }
        }

        pAudioPlayer->resumeSounds();
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
            if (pEventTimer->isPaused())
                dword_6BE364_game_settings_1 |= GAME_SETTINGS_0200_EVENT_TIMER;
            else
                pEventTimer->setPaused(true);
        }

        if (pMiscTimer != nullptr) {
            if (pMiscTimer->isPaused())
                dword_6BE364_game_settings_1 |= GAME_SETTINGS_0400_MISC_TIMER;
            else
                pMiscTimer->setPaused(true);
        }

        if (pAudioPlayer) {
            pAudioPlayer->pauseAllSounds();
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
    engine->config->window.Mode.setValue(mode);
    if (mode == WINDOW_MODE_WINDOWED || mode == WINDOW_MODE_BORDERLESS) {
        window->resize({engine->config->window.Width.value(), engine->config->window.Height.value()});
        window->setPosition(std::get<1>(GetWindowConfigPosition(engine->config.get())));
    }
    render->Reinitialize();
}

void GameWindowHandler::OnToggleResizable() {
    engine->config->window.Resizable.toggle();
    window->setResizable(engine->config->window.Resizable.value());
}

void GameWindowHandler::OnCycleFilter() {
    engine->config->graphics.RenderFilter.cycleIncrement();
    render->Reinitialize();
}

void GameWindowHandler::OnMouseGrabToggle() {
    engine->config->window.MouseGrab.toggle();
    window->setGrabsMouse(engine->config->window.MouseGrab.value());
}

void GameWindowHandler::handleKeyPress(PlatformKey key, PlatformModifiers mods, bool isAutoRepeat) {
    OnChar(key, -1);

    if (!isAutoRepeat) {
        // Do not use autorepeat for game actions, only for text input
        OnKey(key);
    }

    char c = PlatformKeyToChar(key, mods);

    if (c != 0)
        OnChar(key, c);
}

void GameWindowHandler::handleKeyRelease(PlatformKey key) {
    (void) key;
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

bool GameWindowHandler::wheelEvent(const PlatformWheelEvent *event) {
    return false;
}

bool GameWindowHandler::moveEvent(const PlatformMoveEvent *event) {
    /* Remember window position after move. Move position event is also triggered on toggling fullscreen. And we should save current window position prior to entering fullscreen.
     * As entering fullscreen will forcefully move window to {0,0} position on current display. And we want to restore position prior to entering fullscreen and not {0,0} or startup one. */
    PlatformWindowMode mode = window->windowMode();
    if (mode == WINDOW_MODE_WINDOWED || mode == WINDOW_MODE_BORDERLESS) {
        Pointi pos = event->pos;
        auto [display, relativePos, wsize] = GetWindowRelativePosition(&pos);

        engine->config->window.PositionX.setValue(relativePos.x);
        engine->config->window.PositionY.setValue(relativePos.y);
        engine->config->window.Display.setValue(display);
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
    if (_closing)
        return false;
    _closing = true;

    // TODO(captainurist): That's a very convoluted way to exit the game, redo this properly once we have a unified
    //                     event loop.
    dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_APP_INACTIVE;
    component<EngineControlComponent>()->runControlRoutine([] (EngineController *game) {
        game->goToMainMenu();
        game->pressGuiButton("MainMenu_ExitGame");
    });
    return false;
}

bool GameWindowHandler::gamepadConnectionEvent(const PlatformGamepadEvent *event) {
    logger->info("Gamepad {}, model='{}', serial='{}'",
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
        if (key == PlatformKey::KEY_GAMEPAD_LEFTSTICK_RIGHT) {
            key = PlatformKey::KEY_GAMEPAD_LEFTSTICK_LEFT;
        } else if (key == PlatformKey::KEY_GAMEPAD_RIGHTSTICK_RIGHT) {
            key = PlatformKey::KEY_GAMEPAD_RIGHTSTICK_LEFT;
        } else if (key == PlatformKey::KEY_GAMEPAD_LEFTSTICK_DOWN) {
            key = PlatformKey::KEY_GAMEPAD_LEFTSTICK_UP;
        } else if (key == PlatformKey::KEY_GAMEPAD_RIGHTSTICK_DOWN) {
            key = PlatformKey::KEY_GAMEPAD_RIGHTSTICK_UP;
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

bool GameWindowHandler::event(const PlatformEvent *event) {
    if (PlatformEventFilter::event(event))
        return true;

    for (auto &fsmEventHandler : _fsmEventHandlers)
        if (fsmEventHandler->event(event))
            return true;

    return false;
}

void GameWindowHandler::addFsmEventHandler(FsmEventHandler *fsmEventHandler) {
    assert(std::ranges::find(_fsmEventHandlers, fsmEventHandler) == _fsmEventHandlers.end());
    _fsmEventHandlers.push_back(fsmEventHandler);
}

void GameWindowHandler::removeFsmEventHandler(FsmEventHandler *fsmEventHandler) {
    assert(std::ranges::find(_fsmEventHandlers, fsmEventHandler) != _fsmEventHandlers.end());
    std::erase(_fsmEventHandlers, fsmEventHandler);
}
