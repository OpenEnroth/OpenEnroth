#pragma once

#include <SDL.h>

#include "Library/Platform/Interface/PlatformEventLoop.h"
#include "Library/Platform/Interface/PlatformEvents.h"

class SdlPlatformSharedState;

class SdlEventLoop: public PlatformEventLoop {
 public:
    explicit SdlEventLoop(SdlPlatformSharedState *state);
    virtual ~SdlEventLoop();

    virtual void exec(PlatformEventHandler *eventHandler) override;
    virtual void quit() override;

    virtual void processMessages(PlatformEventHandler *eventHandler, int count = -1) override;
    virtual void waitForMessages() override;

 private:
    void dispatchEvent(PlatformEventHandler *eventHandler, const SDL_Event *event);
    void dispatchEvent(PlatformEventHandler *eventHandler, const PlatformEvent *event);
    void dispatchNativeEvent(PlatformEventHandler *eventHandler, const SDL_Event *event);
    void dispatchQuitEvent(PlatformEventHandler *eventHandler, const SDL_QuitEvent *event);
    void dispatchKeyEvent(PlatformEventHandler *eventHandler, const SDL_KeyboardEvent *event);
    void dispatchMouseMoveEvent(PlatformEventHandler *eventHandler, const SDL_MouseMotionEvent *event);
    void dispatchMouseButtonEvent(PlatformEventHandler *eventHandler, const SDL_MouseButtonEvent *event);
    void dispatchMouseWheelEvent(PlatformEventHandler *eventHandler, const SDL_MouseWheelEvent *event);
    void dispatchTouchFingerEvent(PlatformEventHandler *eventHandler, const SDL_TouchFingerEvent *event);
    void dispatchWindowEvent(PlatformEventHandler *eventHandler, const SDL_WindowEvent *event);
    void dispatchWindowMoveEvent(PlatformEventHandler *eventHandler, const SDL_WindowEvent *event);
    void dispatchWindowResizeEvent(PlatformEventHandler *eventHandler, const SDL_WindowEvent *event);
    void dispatchGamepadConnectedEvent(PlatformEventHandler *eventHandler, const SDL_ControllerDeviceEvent *event);
    void dispatchGamepadDisconnectedEvent(PlatformEventHandler *eventHandler, const SDL_ControllerDeviceEvent *event);
    void dispatchGamepadButtonEvent(PlatformEventHandler *eventHandler, const SDL_ControllerButtonEvent *event);
    void dispatchGamepadAxisEvent(PlatformEventHandler *eventHandler, const SDL_ControllerAxisEvent *event);
    void dispatchTextInputEvent(PlatformEventHandler *eventHandler, const SDL_TextInputEvent *event);

 private:
    SdlPlatformSharedState *_state = nullptr;
    bool _quitRequested = false;
};
