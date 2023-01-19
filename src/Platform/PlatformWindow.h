#pragma once

#include <string>
#include <memory>

#include "Utility/Flags.h"
#include "Utility/Geometry/Size.h"
#include "Utility/Geometry/Point.h"
#include "Utility/Geometry/Margins.h"

#include "PlatformOpenGLOptions.h"
#include "PlatformEnums.h"

class PlatformOpenGLContext;


/**
 * Abstraction for accessing platform-specific window API.
 *
 * This class is essentially an API handle and doesn't hold any state. Windows users can think about it as
 * a thin wrapper atop of `HWND`, so a name like `PlatformWindowApiHandle` might've been more suitable here.
 * Fortunately we're not writing in Java and prefer shorter names.
 *
 * Event processing & state storage can be done by subclassing `PlatformEventHandler` and then using it when calling
 * into `PlatformEventLoop`.
 */
class PlatformWindow {
 public:
    virtual ~PlatformWindow() = default;

    virtual void SetTitle(const std::string &title) = 0;
    virtual std::string Title() const = 0;

    virtual void Resize(const Sizei &size) = 0;
    virtual Sizei Size() const = 0;

    virtual void SetPosition(const Pointi &pos) = 0;
    virtual Pointi Position() const = 0;

    virtual void SetVisible(bool visible) = 0;
    virtual bool IsVisible() const = 0;

    virtual void SetResizable(bool resizable) = 0;
    virtual bool Resizable() const = 0;

    virtual void SetWindowMode(PlatformWindowMode mode) = 0;
    virtual PlatformWindowMode WindowMode() = 0;

    virtual void SetGrabsMouse(bool grabsMouse) = 0;
    virtual bool GrabsMouse() const = 0;

    virtual void SetOrientations(PlatformWindowOrientations orientations) = 0;
    virtual PlatformWindowOrientations Orientations() = 0;

    virtual Marginsi FrameMargins() const = 0;

    virtual uintptr_t SystemHandle() const = 0; // TODO(captainurist): docs, @return HWND, etc.

    virtual void Activate() = 0;

    virtual std::unique_ptr<PlatformOpenGLContext> CreateOpenGLContext(const PlatformOpenGLOptions &options) = 0;

#ifdef DDRAW_ENABLED
    // TODO: compat methods, drop alongside with directdraw renderer
    int GetWidth() const {
        return Size().w;
    }

    int GetHeight() const {
        return Size().h;
    }
#endif
};
