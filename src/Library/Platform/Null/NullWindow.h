#pragma once

#include <string>
#include <memory>

#include "Library/Platform/Interface/PlatformWindow.h"

class NullPlatformSharedState;

class NullWindow : public PlatformWindow {
 public:
    explicit NullWindow(NullPlatformSharedState *state);
    virtual ~NullWindow();

    virtual void setTitle(const std::string &title) override;
    virtual std::string title() const override;
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
    virtual void setGrabsMouse(bool grabsMouse) override;
    virtual bool grabsMouse() const override;
    virtual void setOrientations(PlatformWindowOrientations orientations) override;
    virtual PlatformWindowOrientations orientations() override;
    virtual Marginsi frameMargins() const override;
    virtual void *nativeHandle() const override;
    virtual void activate() override;
    virtual std::unique_ptr<PlatformOpenGLContext> createOpenGLContext(const PlatformOpenGLOptions &options) override;

 private:
    NullPlatformSharedState *_state = nullptr;
    std::string _title;
    Pointi  _position;
    Sizei _size;
    bool _visible = true;
    bool _resizable = true;
    PlatformWindowMode _mode = WINDOW_MODE_WINDOWED;
    bool _grabsMouse = false;
    PlatformWindowOrientations _orientations = LANDSCAPE_LEFT | LANDSCAPE_RIGHT | PORTRAIT_UP | PORTRAIT_DOWN;
};
