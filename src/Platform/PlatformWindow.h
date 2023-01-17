#pragma once

#include <string>
#include <memory>

#include "Utility/Flags.h"
#include "Utility/Geometry/Size.h"
#include "Utility/Geometry/Point.h"
#include "Utility/Geometry/Margins.h"

#include "PlatformOpenGLOptions.h"

class PlatformOpenGLContext;

/**
 * - `WINDOWED` is a normal window with frame.
 * - `WINDOWED_BORDERLESS` is a window without frame.
 * - `FULLSCREEN` is a traditional exclusive fullscreen mode. It changes display resolution and makes the window
 *   the sole user of the display.
 * - `FULLSCREEN_BORDERLESS` is a modern fake fullscreen AKA frameless window resized to desktop resolution and moved
 *   to cover the whole desktop.
 */
enum class PlatformWindowMode {
    WINDOWED = 0,
    WINDOWED_BORDERLESS = 1,
    FULLSCREEN = 2,
    FULLSCREEN_BORDERLESS = 3,
};
using enum PlatformWindowMode;

enum class PlatformWindowOrientation {
    LANDSCAPE_LEFT = 0x01,
    LANDSCAPE_RIGHT = 0x02,
    PORTRAIT_UP = 0x04,
    PORTRAIT_DOWN = 0x08,
};
using enum PlatformWindowOrientation;
MM_DECLARE_FLAGS(PlatformWindowOrientations, PlatformWindowOrientation)
MM_DECLARE_OPERATORS_FOR_FLAGS(PlatformWindowOrientations)

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
