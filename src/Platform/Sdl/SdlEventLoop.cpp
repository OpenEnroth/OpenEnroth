#include "SdlEventLoop.h"

#include <SDL.h>

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>
#include <unordered_map>

#include "Platform/PlatformEventHandler.h"
#include "Platform/PlatformEnums.h"

#include "Utility/Segment.h"

#include "SdlPlatformSharedState.h"
#include "SdlEnumTranslation.h"
#include "SdlLogger.h"
#include "SdlWindow.h"
#include "SdlGamepad.h"

SdlEventLoop::SdlEventLoop(SdlPlatformSharedState *state): _state(state) {
    assert(state);
}

SdlEventLoop::~SdlEventLoop() {}

void SdlEventLoop::exec(PlatformEventHandler *eventHandler) {
    assert(eventHandler);

    _quitRequested = false;

    SDL_Event e;
    while (SDL_WaitEvent(&e)) {
        dispatchEvent(eventHandler, &e);

        if (_quitRequested)
            break;
    }
}

void SdlEventLoop::quit() {
    _quitRequested = true;
}

void SdlEventLoop::processMessages(PlatformEventHandler *eventHandler, int count) {
    assert(eventHandler);
    assert(count != 0);

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        dispatchEvent(eventHandler, &e);

        count--;
        if (count == 0)
            break; // Note: count == -1 will never get here, as intended.
    }
}

void SdlEventLoop::waitForMessages() {
    if (SDL_WaitEvent(nullptr) == 0)
        _state->logSdlError("SDL_WaitEvent");
}

void SdlEventLoop::dispatchEvent(PlatformEventHandler *eventHandler, const SDL_Event *event) {
#ifdef MM_PLATFORM_SEND_NATIVE_EVENTS
    dispatchNativeEvent(eventHandler, event);
#endif

    switch(event->type) {
    case SDL_QUIT:
        dispatchQuitEvent(eventHandler, &event->quit);
        break;
    case SDL_KEYUP:
    case SDL_KEYDOWN:
        dispatchKeyEvent(eventHandler, &event->key);
        break;
    case SDL_MOUSEMOTION:
        dispatchMouseMoveEvent(eventHandler, &event->motion);
        break;
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
    case SDL_FINGERUP:
    case SDL_FINGERDOWN:
        dispatchMouseButtonEvent(eventHandler, &event->button);
        break;
    case SDL_MOUSEWHEEL:
        dispatchMouseWheelEvent(eventHandler, &event->wheel);
    case SDL_WINDOWEVENT:
        dispatchWindowEvent(eventHandler, &event->window);
        break;
    case SDL_CONTROLLERDEVICEADDED:
    case SDL_CONTROLLERDEVICEREMOVED:
    case SDL_CONTROLLERDEVICEREMAPPED:
        dispatchGamepadDeviceEvent(eventHandler, &event->cdevice);
        break;
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
        dispatchGamepadButtonEvent(eventHandler, &event->cbutton);
        break;
    case SDL_CONTROLLERAXISMOTION:
        dispatchGamepadAxisEvent(eventHandler, &event->caxis);
        break;
    default:
        break;
    }
}

void SdlEventLoop::dispatchEvent(PlatformEventHandler *eventHandler, const PlatformEvent *event) {
    assert(Segment(PlatformEvent::FirstEventType, PlatformEvent::LastEventType).contains(event->type));

    eventHandler->event(event);
}

void SdlEventLoop::dispatchNativeEvent(PlatformEventHandler *eventHandler, const SDL_Event *event) {
    PlatformNativeEvent e;
    e.type = PlatformEvent::NativeEvent;
    e.nativeEvent = event;

    dispatchEvent(eventHandler, &e);
}

void SdlEventLoop::dispatchQuitEvent(PlatformEventHandler *eventHandler, const SDL_QuitEvent *event) {
    // We don't notify the app of a "termination event", but close all windows instead.
    PlatformWindowEvent e;
    e.type = PlatformEvent::WindowCloseRequest;

    // Saving the ids and not window pointers here is intentional as literally anything could happen
    // inside the event handlers.
    std::vector<uint32_t> windowIds = _state->allWindowIds();
    for (uint32_t id : windowIds) {
        if (PlatformWindow *window = _state->window(id)) { // Window still alive?
            e.window = window;
            dispatchEvent(eventHandler, &e);
        }
    }
}

void SdlEventLoop::dispatchKeyEvent(PlatformEventHandler *eventHandler, const SDL_KeyboardEvent *event) {
    if (event->windowID == 0)
        return; // This happens.

    PlatformKeyEvent e;
    e.window = _state->window(event->windowID);
    e.id = UINT32_MAX; // SDL doesn't discern between multiple keyboards
    e.type = event->type == SDL_KEYUP ? PlatformEvent::KeyRelease : PlatformEvent::KeyPress;
    e.isAutoRepeat = event->repeat;
    e.key = translateSdlKey(event->keysym.scancode);
    e.keyType = KEY_TYPE_KEYBOARD_BUTTON;
    e.keyValue = 0.0f;
    e.mods = translateSdlMods(event->keysym.mod);

    if (e.key != PlatformKey::None)
        dispatchEvent(eventHandler, &e);
}

void SdlEventLoop::dispatchMouseMoveEvent(PlatformEventHandler *eventHandler, const SDL_MouseMotionEvent *event) {
    if (event->windowID == 0)
        return; // This happens.

    PlatformMouseEvent e;
    e.type = PlatformEvent::MouseMove;
    e.window = _state->window(event->windowID);
    e.button = BUTTON_NONE;
    e.buttons = translateSdlMouseButtons(event->state);
    e.isDoubleClick = false;
    e.pos = Pointi(event->x, event->y);
    dispatchEvent(eventHandler, &e);
}

void SdlEventLoop::dispatchMouseButtonEvent(PlatformEventHandler *eventHandler, const SDL_MouseButtonEvent *event) {
    if (event->windowID == 0)
        return; // This happens.

    PlatformMouseEvent e;
    e.type = event->type == SDL_MOUSEBUTTONUP ? PlatformEvent::MouseButtonRelease : PlatformEvent::MouseButtonPress;
    e.window = _state->window(event->windowID);
    e.button = translateSdlMouseButton(event->button);
    e.buttons = translateSdlMouseButtons(SDL_GetMouseState(nullptr, nullptr));
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

    if (e.button != BUTTON_NONE)
        dispatchEvent(eventHandler, &e);
}

void SdlEventLoop::dispatchMouseWheelEvent(PlatformEventHandler *eventHandler, const SDL_MouseWheelEvent *event) {
    if (event->windowID == 0)
        return; // This happens.

    PlatformWheelEvent e;
    e.type = PlatformEvent::MouseWheel;
    e.window = _state->window(event->windowID);
    e.inverted = event->direction == SDL_MOUSEWHEEL_FLIPPED;
    e.angleDelta = {event->x, event->y};
    dispatchEvent(eventHandler, &e);
}

void SdlEventLoop::dispatchWindowEvent(PlatformEventHandler *eventHandler, const SDL_WindowEvent *event) {
    if (event->windowID == 0)
        return; // Shouldn't really happen, but we check just in case.

    PlatformEvent::Type type = PlatformEvent::Invalid;

    switch (event->event) {
    case SDL_WINDOWEVENT_FOCUS_GAINED:
        type = PlatformEvent::WindowActivate;
        break;
    case SDL_WINDOWEVENT_FOCUS_LOST:
        type = PlatformEvent::WindowDeactivate;
        break;
    case SDL_WINDOWEVENT_CLOSE:
        type = PlatformEvent::WindowCloseRequest;
        break;
    case SDL_WINDOWEVENT_MOVED:
        dispatchWindowMoveEvent(eventHandler, event);
        return;
    case SDL_WINDOWEVENT_SIZE_CHANGED:
        // SDL reports window resizes with two events:
        // SDL_WINDOWEVENT_RESIZED - this one is fired only for resizes triggered by user / window manager.
        // SDL_WINDOWEVENT_SIZE_CHANGED - this one is fired every time window size changes, including with a call to
        //                                SDL_SetWindowSize.
        // We obviously need the latter of the two.
        dispatchWindowResizeEvent(eventHandler, event);
        return;
    default:
        return;
    }

    PlatformWindowEvent e;
    e.type = type;
    e.window = _state->window(event->windowID);
    dispatchEvent(eventHandler, &e);
}

void SdlEventLoop::dispatchWindowMoveEvent(PlatformEventHandler *eventHandler, const SDL_WindowEvent *event) {
    assert(event->windowID != 0); // Checked by caller.

    PlatformMoveEvent e;
    e.type = PlatformEvent::WindowMove;
    e.window = _state->window(event->windowID);
    e.pos = {event->data1, event->data2};
    dispatchEvent(eventHandler, &e);
}

void SdlEventLoop::dispatchWindowResizeEvent(PlatformEventHandler *eventHandler, const SDL_WindowEvent *event) {
    assert(event->windowID != 0); // Checked by caller.

    PlatformResizeEvent e;
    e.type = PlatformEvent::WindowResize;
    e.window = _state->window(event->windowID);
    e.size = {event->data1, event->data2};
    dispatchEvent(eventHandler, &e);
}

void SdlEventLoop::dispatchGamepadDeviceEvent(PlatformEventHandler *eventHandler, const SDL_ControllerDeviceEvent *event) {
    PlatformGamepadDeviceEvent e;
    int32_t id = -1;
    if (event->type == SDL_CONTROLLERDEVICEADDED) {
        e.type = PlatformEvent::GamepadConnected;
        id = _state->nextFreeGamepadId();
    } else if (event->type == SDL_CONTROLLERDEVICEREMOVED) {
        e.type = PlatformEvent::GamepadDisconnected;
        id = _state->getGamepadIdBySdlId(event->which);
    } else {
        return;
    }

    assert(id >= 0);
    e.id = id;

    dispatchEvent(eventHandler, &e);
}

void SdlEventLoop::dispatchGamepadButtonEvent(PlatformEventHandler *eventHandler, const SDL_ControllerButtonEvent *event) {
    PlatformKeyEvent e;
    e.id = _state->getGamepadIdBySdlId(event->which);
    e.type = event->type == SDL_CONTROLLERBUTTONUP ? PlatformEvent::KeyRelease : PlatformEvent::KeyPress;
    e.key = translateSdlGamepadButton(static_cast<SDL_GameControllerButton>(event->button));
    e.keyType = KEY_TYPE_GAMEPAD_BUTTON;
    e.keyValue = 0.0f;
    e.isAutoRepeat = false;
    e.mods = 0;

    if (e.key == PlatformKey::None)
        return;

    // TODO(captainurist): separate event type for gamepad events
    std::vector<uint32_t> windowIds = _state->allWindowIds();
    for (uint32_t id : windowIds) {
        if (PlatformWindow *window = _state->window(id)) {
            e.window = window;
            dispatchEvent(eventHandler, &e);
        }
    }
}

void SdlEventLoop::dispatchGamepadAxisEvent(PlatformEventHandler *eventHandler, const SDL_ControllerAxisEvent *event) {
    PlatformKeyEvent e;
    e.id = _state->getGamepadIdBySdlId(event->which);

    // TODO: deadzone should be configurable and default should be lowered once we implement PlatformKeyValue processing.
    int deadzone = 16384;

    /* SDL returns values in range -32768:32767 for axis, and 0:32767 for triggers.
     * Convert that to -1.0:1.0 float fox axis, 0.0:1.0 for triggers and return 0.0 when value is within deadzone. */
    float value = abs(event->value) > deadzone ? std::clamp(event->value / 31767.0f, -1.0f, 1.0f) : 0.0f;

    if (value == 0.0f)
        e.type = PlatformEvent::KeyRelease;
    else
        e.type = PlatformEvent::KeyPress;

    std::pair<PlatformKey, PlatformKeyType> key = translateSdlGamepadAxis(
        static_cast<SDL_GameControllerAxis>(event->axis), event->value);
    e.key = key.first;
    e.keyType = key.second;
    e.keyValue = value;
    e.isAutoRepeat = false;
    e.mods = 0;

    // TODO(captainurist): separate event type for gamepad events
    std::vector<uint32_t> windowIds = _state->allWindowIds();
    for (uint32_t id : windowIds) {
        if (PlatformWindow *window = _state->window(id)) {
            e.window = window;
            dispatchEvent(eventHandler, &e);
        }
    }
}
