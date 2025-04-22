#pragma once

#include <string>
#include <memory>

#include "Library/Geometry/Size.h"
#include "Library/Geometry/Point.h"
#include "Library/Geometry/Margins.h"
#include "Library/Image/Image.h"

#include "Utility/Flags.h"

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

    virtual void setTitle(const std::string &title) = 0;
    virtual std::string title() const = 0;

    virtual void setIcon(RgbaImageView image) = 0;
    // No getter, sorry.

    virtual void resize(const Sizei &size) = 0;
    virtual Sizei size() const = 0;

    virtual void setPosition(const Pointi &pos) = 0;
    virtual Pointi position() const = 0;

    virtual void setVisible(bool visible) = 0;
    virtual bool isVisible() const = 0;

    virtual void setResizable(bool resizable) = 0;
    virtual bool isResizable() const = 0;

    virtual void setWindowMode(PlatformWindowMode mode) = 0;
    virtual PlatformWindowMode windowMode() = 0;

    virtual void setGrabsMouse(bool grabsMouse) = 0;
    virtual bool grabsMouse() const = 0;

    virtual void setOrientations(PlatformWindowOrientations orientations) = 0;
    virtual PlatformWindowOrientations orientations() = 0;

    virtual Marginsi frameMargins() const = 0;

    /**
     * @return                          Pointer to the native window, e.g. `SDL_window` on SDL.
     */
    virtual void *nativeHandle() const = 0;

    virtual void activate() = 0;

    /**
     * Moves the mouse pointer inside this window.
     *
     * Note that this function has no corresponding getter, and this is intentional. Mouse pointer coords are passed
     * in mouse events, and it's the only place to get them from.
     *
     * Also note that this call might generate an `EVENT_MOUSE_MOVE` event.
     *
     * @param position                  Position in window coordinates to move mouse pointer to.
     */
    virtual void warpMouse(Pointi position) = 0;

    virtual std::unique_ptr<PlatformOpenGLContext> createOpenGLContext(const PlatformOpenGLOptions &options) = 0;
};
