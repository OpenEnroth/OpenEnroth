#pragma once

#include <SDL.h>

#include <memory>
#include <string>

#include "Library/Platform/Interface/PlatformWindow.h"

class SdlPlatformSharedState;
class PlatformEventHandler;

class SdlWindow : public PlatformWindow {
 public:
    SdlWindow(SdlPlatformSharedState *state, SDL_Window *window, uint32_t id);
    virtual ~SdlWindow();

    virtual void setTitle(const std::string &title) override;
    virtual std::string title() const override;

    virtual void setIcon(RgbaImageView image) override;

    virtual void resize(const Sizei &size) override;
    virtual Sizei size() const override;

    virtual void setPosition(const Pointi &pos) override;
    virtual Pointi position() const override;

    virtual void setVisible(bool visible) override;
    virtual bool isVisible() const override;

    virtual void setResizable(bool resizable) override;
    virtual bool isResizable() const override;

    virtual void setWindowMode(PlatformWindowMode mode) override;
    virtual PlatformWindowMode windowMode() override;

    virtual void setOrientations(PlatformWindowOrientations orientations) override;
    virtual PlatformWindowOrientations orientations() override;

    virtual void setGrabsMouse(bool grabsMouse) override;
    virtual bool grabsMouse() const override;

    virtual Marginsi frameMargins() const override;

    virtual void *nativeHandle() const override;

    virtual void activate() override;

    virtual void warpMouse(Pointi position) override;

    virtual std::unique_ptr<PlatformOpenGLContext> createOpenGLContext(const PlatformOpenGLOptions &options) override;

    uint32_t id() const {
        return _id;
    }

 private:
    SdlPlatformSharedState *_state = nullptr;
    SDL_Window *_window = nullptr;
    uint32_t _id = 0; // SDL window id, see SDL_GetWindowID.
};
