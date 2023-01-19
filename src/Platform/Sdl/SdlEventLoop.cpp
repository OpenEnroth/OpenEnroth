#include "SdlEventLoop.h"

#include <SDL.h>

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>
#include <unordered_map>

#include "Platform/PlatformEventHandler.h"
#include "Platform/PlatformEnums.h"

#include "SdlPlatformSharedState.h"
#include "SdlEnumTranslation.h"
#include "SdlLogger.h"
#include "SdlWindow.h"
#include "SdlGamepad.h"

SdlEventLoop::SdlEventLoop(SdlPlatformSharedState *state): state_(state) {
    assert(state);
}

SdlEventLoop::~SdlEventLoop() {}

void SdlEventLoop::Exec(PlatformEventHandler *eventHandler) {
    assert(eventHandler);

    quitRequested_ = false;

    SDL_Event e;
    while (SDL_WaitEvent(&e)) {
        DispatchEvent(eventHandler, &e);

        if (quitRequested_)
            break;
    }
}

void SdlEventLoop::Quit() {
    quitRequested_ = true;
}

void SdlEventLoop::ProcessMessages(PlatformEventHandler *eventHandler, int count) {
    assert(eventHandler);
    assert(count != 0);

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        DispatchEvent(eventHandler, &e);

        count--;
        if (count == 0)
            break; // Note: count == -1 will never get here, as intended.
    }
}

void SdlEventLoop::WaitForMessages() {
    if (SDL_WaitEvent(nullptr) == 0)
        state_->LogSdlError("SDL_WaitEvent");
}

void SdlEventLoop::DispatchEvent(PlatformEventHandler *eventHandler, const SDL_Event *event) {
    SDL_GameController *controller = nullptr;
    switch(event->type) {
    case SDL_QUIT:
        DispatchQuitEvent(eventHandler, &event->quit);
        break;
    case SDL_KEYUP:
    case SDL_KEYDOWN:
        DispatchKeyEvent(eventHandler, &event->key);
        break;
    case SDL_MOUSEMOTION:
        DispatchMouseMoveEvent(eventHandler, &event->motion);
        break;
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
    case SDL_FINGERUP:
    case SDL_FINGERDOWN:
        DispatchMouseButtonEvent(eventHandler, &event->button);
        break;
    case SDL_MOUSEWHEEL:
        DispatchMouseWheelEvent(eventHandler, &event->wheel);
    case SDL_WINDOWEVENT:
        DispatchWindowEvent(eventHandler, &event->window);
        break;
    case SDL_CONTROLLERDEVICEADDED:
    case SDL_CONTROLLERDEVICEREMOVED:
    case SDL_CONTROLLERDEVICEREMAPPED:
        DispatchGamepadDeviceEvent(eventHandler, &event->cdevice);
        break;
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
        DispatchGamepadButtonEvent(eventHandler, &event->cbutton);
        break;
    case SDL_CONTROLLERAXISMOTION:
        DispatchGamepadAxisEvent(eventHandler, &event->caxis);
        break;
    default:
        break;
    }
}

void SdlEventLoop::DispatchQuitEvent(PlatformEventHandler *eventHandler, const SDL_QuitEvent *) {
    // We don't notify the app of a "termination event", but close all windows instead.
    PlatformEvent e;
    e.type = PlatformEvent::WindowCloseRequest;

    // Saving the ids and not window pointers here is intentional as literally anything could happen
    // inside the event handlers.
    std::vector<uint32_t> windowIds = state_->AllWindowIds();
    for (uint32_t id : windowIds)
        if (state_->Window(id)) // Window still alive?
            DispatchEvent(eventHandler, id, &e);
}

void SdlEventLoop::DispatchKeyEvent(PlatformEventHandler *eventHandler, const SDL_KeyboardEvent *event) {
    PlatformKeyEvent e;
    e.id = UINT32_MAX; // SDL doesn't discern between multiple keyboards
    e.type = event->type == SDL_KEYUP ? PlatformEvent::KeyRelease : PlatformEvent::KeyPress;
    e.isAutoRepeat = event->repeat;
    e.key = TranslateSdlKey(event->keysym.scancode);
    e.keyType = KEY_TYPE_KEYBOARD_BUTTON;
    e.keyValue = 0.0f;
    e.mods = TranslateSdlMods(event->keysym.mod);

    if (e.key != PlatformKey::None)
        DispatchEvent(eventHandler, event->windowID, &e);
}

void SdlEventLoop::DispatchMouseMoveEvent(PlatformEventHandler *eventHandler, const SDL_MouseMotionEvent *event) {
    PlatformMouseEvent e;
    e.type = PlatformEvent::MouseMove;
    e.button = PlatformMouseButton::None;
    e.buttons = TranslateSdlMouseButtons(event->state);
    e.isDoubleClick = false;
    e.pos = Pointi(event->x, event->y);
    DispatchEvent(eventHandler, event->windowID, &e);
}

void SdlEventLoop::DispatchMouseButtonEvent(PlatformEventHandler *eventHandler, const SDL_MouseButtonEvent *event) {
    PlatformMouseEvent e;
    e.type = event->type == SDL_MOUSEBUTTONUP ? PlatformEvent::MouseButtonRelease : PlatformEvent::MouseButtonPress;
    e.button = TranslateSdlMouseButton(event->button);
    e.buttons = TranslateSdlMouseButtons(SDL_GetMouseState(nullptr, nullptr));
    e.pos = Pointi(event->x, event->y);

    if (event->type == SDL_MOUSEBUTTONUP) {
        e.isDoubleClick = false;
    } else {
#ifndef __ANDROID__
        // Number of clicks as reported by SDL starts at 1, and by clicking repeatedly you can get this number
        // literally to 100s. We just treat every 2nd click as a double click.
        assert(event->clicks > 0);
        e.isDoubleClick = (event->clicks % 2 == 0);
#else
        // TODO: on Android it seems to start with 0.
        e.isDoubleClick = (event->clicks % 2 == 1);
#endif
    }

    if (e.button != PlatformMouseButton::None)
        DispatchEvent(eventHandler, event->windowID, &e);
}

void SdlEventLoop::DispatchMouseWheelEvent(PlatformEventHandler *eventHandler, const SDL_MouseWheelEvent *event) {
    PlatformWheelEvent e;
    e.type = PlatformEvent::MouseWheel;
    e.inverted = event->direction == SDL_MOUSEWHEEL_FLIPPED;
    e.angleDelta = {event->x, event->y};
    DispatchEvent(eventHandler, event->windowID, &e);
}

void SdlEventLoop::DispatchWindowEvent(PlatformEventHandler *eventHandler, const SDL_WindowEvent *event) {
    switch (event->event) {
    case SDL_WINDOWEVENT_FOCUS_GAINED:
        DispatchEvent(eventHandler, event->windowID, PlatformEvent::WindowActivate);
        return;
    case SDL_WINDOWEVENT_FOCUS_LOST:
        DispatchEvent(eventHandler, event->windowID, PlatformEvent::WindowDeactivate);
        return;
    case SDL_WINDOWEVENT_CLOSE:
        DispatchEvent(eventHandler, event->windowID, PlatformEvent::WindowCloseRequest);
        return;
    case SDL_WINDOWEVENT_MOVED:
        DispatchWindowMoveEvent(eventHandler, event);
        return;
    case SDL_WINDOWEVENT_RESIZED:
        DispatchWindowResizeEvent(eventHandler, event);
        return;
    default:
        return;
    }
}

void SdlEventLoop::DispatchWindowMoveEvent(PlatformEventHandler *eventHandler, const SDL_WindowEvent *event) {
    PlatformMoveEvent e;
    e.type = PlatformEvent::WindowMove;
    e.pos = {event->data1, event->data2};
    DispatchEvent(eventHandler, event->windowID, &e);
}

void SdlEventLoop::DispatchWindowResizeEvent(PlatformEventHandler *eventHandler, const SDL_WindowEvent *event) {
    PlatformResizeEvent e;
    e.type = PlatformEvent::WindowResize;
    e.size = {event->data1, event->data2};
    DispatchEvent(eventHandler, event->windowID, &e);
}

void SdlEventLoop::DispatchEvent(PlatformEventHandler *eventHandler, uint32_t windowId, PlatformEvent::Type type) {
    PlatformEvent e;
    e.type = type;
    DispatchEvent(eventHandler, windowId, &e);
}

void SdlEventLoop::DispatchEvent(PlatformEventHandler *eventHandler, uint32_t windowId, PlatformEvent *event) {
    assert(event->type != PlatformEvent::Invalid);

    if (windowId == 0)
        return; // This does happen, 0 means 'no window'.

    eventHandler->Event(state_->Window(windowId), event);
}

void SdlEventLoop::DispatchGamepadDeviceEvent(PlatformEventHandler *eventHandler, const SDL_ControllerDeviceEvent *event) {
    PlatformGamepadDeviceEvent e;
    int32_t id = -1;
    if (event->type == SDL_CONTROLLERDEVICEADDED) {
        e.type = PlatformEvent::GamepadConnected;
        id = state_->NextFreeGamepadId();
    } else if (event->type == SDL_CONTROLLERDEVICEREMOVED) {
        e.type = PlatformEvent::GamepadDisconnected;
        id = state_->GetGamepadIdBySdlId(event->which);
    } else {
        return;
    }

    assert(id >= 0);
    e.id = id;

    std::vector<uint32_t> windowIds = state_->AllWindowIds();
    for (uint32_t id : windowIds)
        if (state_->Window(id)) // Window still alive?
            DispatchEvent(eventHandler, id, &e);
}

void SdlEventLoop::DispatchGamepadButtonEvent(PlatformEventHandler *eventHandler, const SDL_ControllerButtonEvent *event) {
    PlatformKeyEvent e;
    e.id = state_->GetGamepadIdBySdlId(event->which);
    e.type = event->type == SDL_CONTROLLERBUTTONUP ? PlatformEvent::KeyRelease : PlatformEvent::KeyPress;
    e.key = TranslateSdlGamepadButton(static_cast<SDL_GameControllerButton>(event->button));
    e.keyType = KEY_TYPE_GAMEPAD_BUTTON;
    e.keyValue = 0.0f;
    e.isAutoRepeat = false;
    e.mods = 0;

    if (e.key == PlatformKey::None)
        return;

    std::vector<uint32_t> windowIds = state_->AllWindowIds();
    for (uint32_t id : windowIds)
        if (state_->Window(id)) // Window still alive?
            DispatchEvent(eventHandler, id, &e);
}

void SdlEventLoop::DispatchGamepadAxisEvent(PlatformEventHandler *eventHandler, const SDL_ControllerAxisEvent *event) {
    PlatformKeyEvent e;
    e.id = state_->GetGamepadIdBySdlId(event->which);

    // TODO: deadzone should be configurable and default should be lowered once we implement PlatformKeyValue processing.
    int deadzone = 16384;

    /* SDL returns values in range -32768:32767 for axis, and 0:32767 for triggers.
     * Convert that to -1.0:1.0 float fox axis, 0.0:1.0 for triggers and return 0.0 when value is within deadzone. */
    float value = abs(event->value) > deadzone ? std::clamp(event->value / 31767.0f, -1.0f, 1.0f) : 0.0f;

    if (value == 0.0f)
        e.type = PlatformEvent::KeyRelease;
    else
        e.type = PlatformEvent::KeyPress;

    std::pair<PlatformKey, PlatformKeyType> key = TranslateSdlGamepadAxis(static_cast<SDL_GameControllerAxis>(event->axis), event->value);
    e.key = key.first;
    e.keyType = key.second;
    e.keyValue = value;
    e.isAutoRepeat = false;
    e.mods = 0;

    std::vector<uint32_t> windowIds = state_->AllWindowIds();
    for (uint32_t id : windowIds)
        if (state_->Window(id)) // Window still alive?
            DispatchEvent(eventHandler, id, &e);
}
