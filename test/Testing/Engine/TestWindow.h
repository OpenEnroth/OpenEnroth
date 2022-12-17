#pragma once

#include <memory>
#include <string>

#include "Platform/PlatformWindow.h"

#include "TestStateHandle.h"

class TestWindow : public PlatformWindow {
 public:
    TestWindow(std::unique_ptr<PlatformWindow> base, TestStateHandle state);
    virtual ~TestWindow();

    virtual void SetTitle(const std::string &title) override;
    virtual std::string Title() const override;

    virtual void Resize(const Sizei &size) override;
    virtual Sizei Size() const override;

    virtual void SetPosition(const Pointi &pos) override;
    virtual Pointi Position() const override;

    virtual void SetVisible(bool visible) override;
    virtual bool IsVisible() const override;

    virtual void SetWindowMode(PlatformWindowMode mode) override;
    virtual PlatformWindowMode WindowMode() override;

    virtual void SetGrabsMouse(bool grabsMouse) override;
    virtual bool GrabsMouse() const override;

    virtual Marginsi FrameMargins() const override;

    virtual uintptr_t SystemHandle() const override;

    virtual void Activate() override;

    virtual std::unique_ptr<PlatformOpenGLContext> CreateOpenGLContext(const PlatformOpenGLOptions &options) override;

 private:
    std::unique_ptr<PlatformWindow> base_;
    TestStateHandle state_;
};
