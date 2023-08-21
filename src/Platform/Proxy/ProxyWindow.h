#pragma once

#include <stdint.h>
#include <string>
#include <memory>

#include "Platform/PlatformWindow.h"
#include "ProxyBase.h"
#include "Platform/PlatformEnums.h"
#include "Utility/Geometry/Margins.h"
#include "Utility/Geometry/Point.h"
#include "Utility/Geometry/Size.h"

class PlatformOpenGLContext;
class PlatformWindow;
struct PlatformOpenGLOptions;

class ProxyWindow : public ProxyBase<PlatformWindow> {
 public:
    explicit ProxyWindow(PlatformWindow *base = nullptr);
    virtual ~ProxyWindow() = default;

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
    virtual uintptr_t systemHandle() const override;
    virtual void activate() override;
    virtual std::unique_ptr<PlatformOpenGLContext> createOpenGLContext(const PlatformOpenGLOptions &options) override;
};
