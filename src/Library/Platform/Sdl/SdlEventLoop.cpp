#include "SdlEventLoop.h"

#include <SDL.h>

#include <algorithm>
#include <cassert>
#include <vector>

#include "Library/Platform/Interface/PlatformEventHandler.h"
#include "Library/Platform/Interface/PlatformEnums.h"

#include "Utility/Segment.h"
#include "Utility/ScopeGuard.h"

#include "SdlPlatformSharedState.h"
#include "SdlEnumTranslation.h"
#include "SdlWindow.h"
#include "SdlGamepad.h"

SdlEventLoop::SdlEventLoop(SdlPlatformSharedState *state): _state(state) {
    assert(state);
    _state->registerEventLoop(this);
}

SdlEventLoop::~SdlEventLoop() {
    _state->unregisterEventLoop(this);
}

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
    dispatchNativeEvent(eventHandler, event);

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
        dispatchMouseButtonEvent(eventHandler, &event->button);
        break;
    case SDL_MOUSEWHEEL:
        dispatchMouseWheelEvent(eventHandler, &event->wheel);
    case SDL_FINGERUP:
    case SDL_FINGERDOWN:
    case SDL_FINGERMOTION:
        dispatchTouchFingerEvent(eventHandler, &event->tfinger);
        break;
    case SDL_WINDOWEVENT:
        dispatchWindowEvent(eventHandler, &event->window);
        break;
    case SDL_CONTROLLERDEVICEADDED:
        dispatchGamepadConnectedEvent(eventHandler, &event->cdevice);
        break;
    case SDL_CONTROLLERDEVICEREMOVED:
        dispatchGamepadDisconnectedEvent(eventHandler, &event->cdevice);
        break;
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
        dispatchGamepadButtonEvent(eventHandler, &event->cbutton);
        break;
    case SDL_CONTROLLERAXISMOTION:
        dispatchGamepadAxisEvent(eventHandler, &event->caxis);
        break;
    case SDL_TEXTINPUT:
        dispatchTextInputEvent(eventHandler, &event->text);
        break;
    default:
        break;
    }
}

void SdlEventLoop::dispatchEvent(PlatformEventHandler *eventHandler, const PlatformEvent *event) {
    assert(Segment(EVENT_FIRST, EVENT_LAST).contains(event->type));

    eventHandler->event(event);
}

void SdlEventLoop::dispatchNativeEvent(PlatformEventHandler *eventHandler, const SDL_Event *event) {
    PlatformNativeEvent e;
    e.type = EVENT_NATIVE;
    e.nativeEvent = event;

    dispatchEvent(eventHandler, &e);
}

void SdlEventLoop::dispatchQuitEvent(PlatformEventHandler *eventHandler, const SDL_QuitEvent *event) {
    // We don't notify the app of a "termination event", but close all windows instead.
    PlatformWindowEvent e;
    e.type = EVENT_WINDOW_CLOSE_REQUEST;

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
    e.type = event->type == SDL_KEYUP ? EVENT_KEY_RELEASE : EVENT_KEY_PRESS;
    e.isAutoRepeat = event->repeat;
    e.key = translateSdlKey(event->keysym.scancode);
    e.mods = translateSdlMods(event->keysym.mod);

    if (e.key != PlatformKey::KEY_NONE)
        dispatchEvent(eventHandler, &e);
}

void SdlEventLoop::dispatchMouseMoveEvent(PlatformEventHandler *eventHandler, const SDL_MouseMotionEvent *event) {
    if (event->windowID == 0)
        return; // This happens.

    PlatformMouseEvent e;
    e.type = EVENT_MOUSE_MOVE;
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
    e.type = event->type == SDL_MOUSEBUTTONUP ? EVENT_MOUSE_BUTTON_RELEASE : EVENT_MOUSE_BUTTON_PRESS;
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
        // On Android it seems to start with 0.
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
    e.type = EVENT_MOUSE_WHEEL;
    e.window = _state->window(event->windowID);
    // SDL inverts event->y for us when event->direction == SDL_MOUSEWHEEL_FLIPPED, so we don't need to check for it.
    e.angleDelta = {event->x, event->y};
    dispatchEvent(eventHandler, &e);
}

void SdlEventLoop::dispatchTouchFingerEvent(PlatformEventHandler *eventHandler, const SDL_TouchFingerEvent *event) {
    if (event->windowID == 0)
        return; // This happens.

    PlatformMouseEvent e;
    switch (event->type) {
    case SDL_FINGERDOWN:
        e.type = EVENT_MOUSE_BUTTON_PRESS;
        e.button = BUTTON_LEFT;
        e.buttons = BUTTON_LEFT; // Not calling SDL_GetMouseState here because if we want buttons to sync up properly
                                 // then we'd also need to get touch state from inside the mouse event.
        break;
    case SDL_FINGERUP:
        e.type = EVENT_MOUSE_BUTTON_RELEASE;
        e.button = BUTTON_LEFT;
        break;
    case SDL_FINGERMOTION:
        e.type = EVENT_MOUSE_MOVE;
        break;
    default:
        return;
    }
    e.window = _state->window(event->windowID);
    e.pos = Pointi(event->x * e.window->size().w, event->y * e.window->size().h);

    dispatchEvent(eventHandler, &e);
}

void SdlEventLoop::dispatchWindowEvent(PlatformEventHandler *eventHandler, const SDL_WindowEvent *event) {
    if (event->windowID == 0)
        return; // Shouldn't really happen, but we check just in case.

    PlatformEventType type = EVENT_INVALID;

    switch (event->event) {
    case SDL_WINDOWEVENT_FOCUS_GAINED:
        type = EVENT_WINDOW_ACTIVATE;
        break;
    case SDL_WINDOWEVENT_FOCUS_LOST:
        type = EVENT_WINDOW_DEACTIVATE;
        break;
    case SDL_WINDOWEVENT_CLOSE:
        type = EVENT_WINDOW_CLOSE_REQUEST;
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
    e.type = EVENT_WINDOW_MOVE;
    e.window = _state->window(event->windowID);
    e.pos = {event->data1, event->data2};
    dispatchEvent(eventHandler, &e);
}

void SdlEventLoop::dispatchWindowResizeEvent(PlatformEventHandler *eventHandler, const SDL_WindowEvent *event) {
    assert(event->windowID != 0); // Checked by caller.

    PlatformResizeEvent e;
    e.type = EVENT_WINDOW_RESIZE;
    e.window = _state->window(event->windowID);
    e.size = {event->data1, event->data2};
    dispatchEvent(eventHandler, &e);
}

void SdlEventLoop::dispatchGamepadConnectedEvent(PlatformEventHandler *eventHandler, const SDL_ControllerDeviceEvent *event) {
    PlatformGamepadEvent e;
    e.type = EVENT_GAMEPAD_CONNECTED;
    e.gamepad = _state->initializeGamepad(event->which);
    if (!e.gamepad)
        return;

    dispatchEvent(eventHandler, &e);
}

void SdlEventLoop::dispatchGamepadDisconnectedEvent(PlatformEventHandler *eventHandler, const SDL_ControllerDeviceEvent *event) {
    PlatformGamepadEvent e;
    e.type = EVENT_GAMEPAD_DISCONNECTED;
    e.gamepad = _state->gamepad(event->which);
    if (!e.gamepad)
        return;

    MM_AT_SCOPE_EXIT(_state->deinitializeGamepad(event->which));

    dispatchEvent(eventHandler, &e);
}

void SdlEventLoop::dispatchGamepadButtonEvent(PlatformEventHandler *eventHandler, const SDL_ControllerButtonEvent *event) {
    PlatformGamepadKeyEvent e;
    e.gamepad = _state->gamepad(event->which);
    e.type = event->type == SDL_CONTROLLERBUTTONUP ? EVENT_GAMEPAD_KEY_RELEASE : EVENT_GAMEPAD_KEY_PRESS;
    e.key = translateSdlGamepadButton(static_cast<SDL_GameControllerButton>(event->button));

    if (!e.gamepad || e.key == PlatformKey::KEY_NONE)
        return;

    dispatchEvent(eventHandler, &e);
}

void SdlEventLoop::dispatchGamepadAxisEvent(PlatformEventHandler *eventHandler, const SDL_ControllerAxisEvent *event) {
    PlatformGamepadAxisEvent e;
    e.gamepad = _state->gamepad(event->which);
    e.type = EVENT_GAMEPAD_AXIS;
    e.axis = translateSdlGamepadAxis(static_cast<SDL_GameControllerAxis>(event->axis));
    e.value = std::clamp(event->value / 31767.0f, -1.0f, 1.0f);

    if (!e.gamepad || e.axis == PlatformKey::KEY_NONE)
        return;

    dispatchEvent(eventHandler, &e);
}

void SdlEventLoop::dispatchTextInputEvent(PlatformEventHandler* eventHandler, const SDL_TextInputEvent* event) {
    PlatformTextInputEvent e;
    e.type = EVENT_TEXT_INPUT;
    e.window = _state->window(event->windowID);
    e.text = event->text;

    dispatchEvent(eventHandler, &e);
}
