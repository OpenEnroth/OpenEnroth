#include "SdlEventLoop.h"

#include <SDL.h>

#include <cstdint>
#include <cassert>

#include "Platform/PlatformEvents.h"
#include "Utility/ScopeGuard.h"

#include "SdlPlatformSharedState.h"
#include "SdlEnumTranslation.h"

SdlEventLoop::SdlEventLoop(SdlPlatformSharedState *state): state_(state) {
    assert(state);
}

SdlEventLoop::~SdlEventLoop() {}

void SdlEventLoop::Exec() {
    if (state_->IsTerminating())
        return;

    quitRequested_ = false;

    SDL_Event e;
    while (SDL_WaitEvent(&e)) {
        DispatchEvent(&e);

        if (quitRequested_ || state_->IsTerminating())
            break;
    }
}

void SdlEventLoop::Quit() {
    quitRequested_ = true;
}

void SdlEventLoop::ProcessMessages(int count) {
    assert(count != 0);

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        DispatchEvent(&e);

        if (state_->IsTerminating()) // We don't check for quitRequested_ here as it's only for leaving Exec.
            break;

        count--;
        if (count == 0)
            break; // Note: count == -1 will never get here, as intended.
    }
}

void SdlEventLoop::WaitForMessages() {
    if (SDL_WaitEvent(nullptr) == 0)
        state_->LogSdlError("SDL_WaitEvent");
}

void SdlEventLoop::DispatchEvent(const SDL_Event *event) {
    switch(event->type) {
    case SDL_QUIT:
        DispatchQuitEvent(&event->quit);
        break;
    case SDL_KEYUP:
    case SDL_KEYDOWN:
        DispatchKeyEvent(&event->key);
        break;
    case SDL_MOUSEMOTION:
        DispatchMouseMoveEvent(&event->motion);
        break;
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
        DispatchMouseButtonEvent(&event->button);
        break;
    case SDL_MOUSEWHEEL:
        DispatchMouseWheelEvent(&event->wheel);
    case SDL_WINDOWEVENT:
        DispatchWindowEvent(&event->window);
    default:
        break;
    }
}

void SdlEventLoop::DispatchQuitEvent(const SDL_QuitEvent *) {
    state_->SetTerminating(true);
    // TODO(captainurist): do we need to manually close all windows?
}

void SdlEventLoop::DispatchKeyEvent(const SDL_KeyboardEvent *event) {
    PlatformKeyEvent e;
    e.type = event->type == SDL_KEYUP ? PlatformEvent::KeyRelease : PlatformEvent::KeyPress;
    e.isAutoRepeat = event->repeat;
    e.key = TranslateSdlKey(event->keysym.scancode);
    e.mods = TranslateSdlMods(event->keysym.mod);

    if (e.key != PlatformKey::None)
        state_->SendEvent(event->windowID, &e);
}

void SdlEventLoop::DispatchMouseMoveEvent(const SDL_MouseMotionEvent *event) {
    PlatformMouseEvent e;
    e.type = PlatformEvent::MouseMove;
    e.button = PlatformMouseButton::None;
    e.buttons = TranslateSdlMouseButtons(event->state);
    e.isDoubleClick = false;
    e.pos = Pointi(event->x, event->y);
    state_->SendEvent(event->windowID, &e);
}

void SdlEventLoop::DispatchMouseButtonEvent(const SDL_MouseButtonEvent *event) {
    PlatformMouseEvent e;
    e.type = event->type == SDL_MOUSEBUTTONUP ? PlatformEvent::MouseButtonRelease : PlatformEvent::MouseButtonPress;
    e.button = TranslateSdlMouseButton(event->button);
    e.buttons = TranslateSdlMouseButtons(SDL_GetMouseState(nullptr, nullptr)); // TODO(captainurist) : this actually needs testing.
    e.isDoubleClick = (event->clicks % 2 == 0); // TODO: and this
    e.pos = Pointi(event->x, event->y);

    if (e.button != PlatformMouseButton::None)
        state_->SendEvent(event->windowID, &e);
}

void SdlEventLoop::DispatchMouseWheelEvent(const SDL_MouseWheelEvent *event) {
    PlatformWheelEvent e;
    e.type = PlatformEvent::MouseWheel;
    e.inverted = event->direction == SDL_MOUSEWHEEL_FLIPPED;
    e.angleDelta = {event->x, event->y};
    state_->SendEvent(event->windowID, &e);
}

void SdlEventLoop::DispatchWindowEvent(const SDL_WindowEvent *event) {
    switch (event->event) {
    case SDL_WINDOWEVENT_FOCUS_GAINED: {
        PlatformEvent e;
        e.type = PlatformEvent::WindowActivated;
        state_->SendEvent(event->windowID, &e);
        break;
    }
    case SDL_WINDOWEVENT_FOCUS_LOST: {
        PlatformEvent e;
        e.type = PlatformEvent::WindowDeactivated;
        state_->SendEvent(event->windowID, &e);
        break;
    }
    default:
        break;
    }
}


