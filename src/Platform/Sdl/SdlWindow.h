#pragma once

#include <SDL.h>

#include <memory>
#include <string>

#include "Platform/PlatformWindow.h"

class Log;
class SdlPlatformSharedState;
class PlatformEventHandler;

class SdlWindow : public PlatformWindow {
 public:
    SdlWindow(SdlPlatformSharedState *state, std::unique_ptr<PlatformEventHandler> eventHandler, SDL_Window *window, uint32_t id);
    virtual ~SdlWindow();

    virtual void SetTitle(const std::string &title) override;
    virtual std::string Title() const override;

    virtual void Resize(const Sizei &size) override;
    virtual Sizei Size() const override;

    virtual void SetPosition(const Pointi &pos) override;
    virtual Pointi Position() const override;

    virtual void SetVisible(bool visible) override;
    virtual bool IsVisible() const override;

    virtual void SetFullscreen(bool fullscreen) override;
    virtual bool IsFullscreen() const override;

    virtual void SetFrameless(bool frameless) override;
    virtual bool IsFrameless() const override;

    virtual void SetGrabsMouse(bool grabsMouse) override;
    virtual bool GrabsMouse() const override;

    virtual void Activate() override;

    virtual uintptr_t SystemHandle() const override;

    virtual std::unique_ptr<PlatformOpenGLContext> CreateOpenGLContext(const PlatformOpenGLOptions &options) override;

    uint32_t Id() const {
        return id_;
    }

    SDL_Window *SdlHandle() const {
        return window_;
    }

    PlatformEventHandler *EventHandler() const {
        return eventHandler_.get();
    }

 private:
    SdlPlatformSharedState *state_ = nullptr;
    std::unique_ptr<PlatformEventHandler> eventHandler_;
    SDL_Window *window_ = nullptr;
    uint32_t id_ = 0;
};
