#pragma once

#include <SDL.h>

#include "Platform/PlatformEventLoop.h"

class SdlPlatformSharedState;

class SdlEventLoop: public PlatformEventLoop {
 public:
    SdlEventLoop(SdlPlatformSharedState *state);
    virtual ~SdlEventLoop();

    virtual void Exec() override;
    virtual void Quit() override;

    virtual void ProcessMessages(int count = -1) override;
    virtual void WaitForMessages() override;

 private:
    void DispatchEvent(const SDL_Event *event);
    void DispatchQuitEvent(const SDL_QuitEvent *event);
    void DispatchKeyEvent(const SDL_KeyboardEvent *event);
    void DispatchMouseMoveEvent(const SDL_MouseMotionEvent *event);
    void DispatchMouseButtonEvent(const SDL_MouseButtonEvent *event);
    void DispatchMouseWheelEvent(const SDL_MouseWheelEvent *event);
    void DispatchWindowEvent(const SDL_WindowEvent *event);

 private:
    SdlPlatformSharedState *state_ = nullptr;
    bool quitRequested_ = false;
};
