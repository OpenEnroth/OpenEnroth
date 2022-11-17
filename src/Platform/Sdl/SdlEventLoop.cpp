#include "SdlEventLoop.h"

#include <SDL.h>

#include <cassert>
#include <vector>

#include "Platform/PlatformEvents.h"
#include "Platform/PlatformEventHandler.h"

#include "SdlPlatformSharedState.h"
#include "SdlEnumTranslation.h"
#include "SdlWindow.h"

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
        DispatchMouseButtonEvent(eventHandler, &event->button);
        break;
    case SDL_MOUSEWHEEL:
        DispatchMouseWheelEvent(eventHandler, &event->wheel);
    case SDL_WINDOWEVENT:
        DispatchWindowEvent(eventHandler, &event->window);
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
    e.type = event->type == SDL_KEYUP ? PlatformEvent::KeyRelease : PlatformEvent::KeyPress;
    e.isAutoRepeat = event->repeat;
    e.key = TranslateSdlKey(event->keysym.scancode);
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

    // Number of clicks as reported by SDL starts at 1, and by clicking repeatedly you can get this number
    // literally to 100s. We just treat every 2nd click as a double click.
    assert(event->clicks > 0);
    e.isDoubleClick = (event->clicks % 2 == 0);

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

void SdlEventLoop::DispatchEvent(PlatformEventHandler *eventHandler, uint32_t windowId, PlatformEvent::Type type) {
    PlatformEvent e;
    e.type = type;
    DispatchEvent(eventHandler, windowId, &e);
}

void SdlEventLoop::DispatchEvent(PlatformEventHandler *eventHandler, uint32_t windowId, PlatformEvent *event) {
    assert(event->type != PlatformEvent::Invalid);

    eventHandler->Event(state_->Window(windowId), event);
}

