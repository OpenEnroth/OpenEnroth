#pragma once

#include <SDL.h>

#include "Platform/PlatformEventLoop.h"
#include "Platform/PlatformEvents.h"

class SdlPlatformSharedState;

class SdlEventLoop: public PlatformEventLoop {
 public:
    explicit SdlEventLoop(SdlPlatformSharedState *state);
    virtual ~SdlEventLoop();

    virtual void Exec(PlatformEventHandler *eventHandler) override;
    virtual void Quit() override;

    virtual void ProcessMessages(PlatformEventHandler *eventHandler, int count = -1) override;
    virtual void WaitForMessages() override;

 private:
    void DispatchEvent(PlatformEventHandler *eventHandler, const SDL_Event *event);
    void DispatchNativeEvent(PlatformEventHandler *eventHandler, const SDL_Event *event);
    void DispatchQuitEvent(PlatformEventHandler *eventHandler, const SDL_QuitEvent *event);
    void DispatchKeyEvent(PlatformEventHandler *eventHandler, const SDL_KeyboardEvent *event);
    void DispatchMouseMoveEvent(PlatformEventHandler *eventHandler, const SDL_MouseMotionEvent *event);
    void DispatchMouseButtonEvent(PlatformEventHandler *eventHandler, const SDL_MouseButtonEvent *event);
    void DispatchMouseWheelEvent(PlatformEventHandler *eventHandler, const SDL_MouseWheelEvent *event);
    void DispatchWindowEvent(PlatformEventHandler *eventHandler, const SDL_WindowEvent *event);
    void DispatchWindowMoveEvent(PlatformEventHandler *eventHandler, const SDL_WindowEvent *event);
    void DispatchWindowResizeEvent(PlatformEventHandler *eventHandler, const SDL_WindowEvent *event);
    void DispatchEvent(PlatformEventHandler *eventHandler, uint32_t windowId, PlatformEvent::Type type);
    void DispatchEvent(PlatformEventHandler *eventHandler, uint32_t windowId, PlatformEvent *event);
    void DispatchGamepadDeviceEvent(PlatformEventHandler *eventHandler, const SDL_ControllerDeviceEvent *event);
    void DispatchGamepadButtonEvent(PlatformEventHandler *eventHandler, const SDL_ControllerButtonEvent *event);
    void DispatchGamepadAxisEvent(PlatformEventHandler *eventHandler, const SDL_ControllerAxisEvent *event);

 private:
    SdlPlatformSharedState *state_ = nullptr;
    bool quitRequested_ = false;
};
