#pragma once

#include <string>
#include <memory>

#include "Platform/PlatformWindow.h"

#include "ProxyBase.h"

class ProxyWindow : public ProxyBase<PlatformWindow> {
 public:
    explicit ProxyWindow(PlatformWindow *base = nullptr);
    virtual ~ProxyWindow() = default;

    virtual void SetTitle(const std::string &title) override;
    virtual std::string Title() const override;
    virtual void Resize(const Sizei &size) override;
    virtual Sizei Size() const override;
    virtual void SetPosition(const Pointi &pos) override;
    virtual Pointi Position() const override;
    virtual void SetVisible(bool visible) override;
    virtual bool IsVisible() const override;
    virtual void SetResizable(bool resizable) override;
    virtual bool Resizable() const override;
    virtual void SetWindowMode(PlatformWindowMode mode) override;
    virtual PlatformWindowMode WindowMode() override;
    virtual void SetGrabsMouse(bool grabsMouse) override;
    virtual bool GrabsMouse() const override;
    virtual void SetOrientations(PlatformWindowOrientations orientations) override;
    virtual PlatformWindowOrientations Orientations() override;
    virtual Marginsi FrameMargins() const override;
    virtual uintptr_t SystemHandle() const override;
    virtual void Activate() override;
    virtual std::unique_ptr<PlatformOpenGLContext> CreateOpenGLContext(const PlatformOpenGLOptions &options) override;
};
