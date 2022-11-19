#pragma once

#include <memory>
#include <string>

#include "Platform/PlatformWindow.h"

#include "GameTestStateHandle.h"

class GameTestWindow : public PlatformWindow {
 public:
    GameTestWindow(std::unique_ptr<PlatformWindow> base, GameTestStateHandle state);
    virtual ~GameTestWindow();

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

    virtual void SetBorderless(bool borderless) override;
    virtual bool IsBorderless() const override;

    virtual void SetGrabsMouse(bool grabsMouse) override;
    virtual bool GrabsMouse() const override;

    virtual void Activate() override;

    virtual uintptr_t SystemHandle() const override;

    virtual std::unique_ptr<PlatformOpenGLContext> CreateOpenGLContext(const PlatformOpenGLOptions &options) override;

 private:
    std::unique_ptr<PlatformWindow> base_;
    GameTestStateHandle state_;
};
