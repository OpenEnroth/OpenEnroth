#pragma once

#include <memory>
#include <vector>

#include "Utility/Geometry/Rect.h"

#ifdef CreateWindow
#   undef CreateWindow
#endif

class Log;
class PlatformEvent;
class PlatformWindow;
class PlatformEventLoop;
class PlatformEventHandler;

/**
 * Platform abstraction layer.
 *
 * Platform classes can be broadly divided into three categories:
 * - API handles. These classes don't hold any state and cannot be subclassed in user code.
 * - Extension points. These classes can hold state and can be subclassed in user code.
 * - Value types.
 *
 * API handles exposed by the platform are:
 * - `Platform`, which also works as a factory for other API handles.
 * - `PlatformWindow`, an API handle for a GUI window.
 * - `PlatformEventLoop`, an API handle for running an event loop.
 * - `PlatformOpenGLContext`, an API handle for accessing OpenGL API for a window.
 *
 * Then there are the following extension points:
 * - `PlatformEventHandler`, which should be subclassed in user code to handle platform events. An instance is passed
 *   to `PlatformEventLoop` methods, so this is where the calls back into the user code happen.
 *
 * That's it, there is only one extension point. If you want fancy event handler chains, you can do this in client code.
 *
 * And then there are value types:
 * - All kinds of enums for keyboard keys, mouse keys, and modifiers.
 * - `PlatformEvent` and subclasses for delivering events to `PlatformEventHandler`.
 *
 * All platform classes don't throw but write errors to log instead. See per-method docs for details on how errors
 * are reported by each method.
 *
 * Also platform declares its own `main` function, so you cannot declare `main` in your code. Platform expects you
 * to define `PlatformMain` instead.
 */
class Platform {
 public:
    virtual ~Platform() = default;

    /**
     * Creates a standard platform.
     *
     * @param log                       Log stream to use. Must not be `nullptr`.
     * @return                          A newly created `Platform`. This method is guaranteed to succeed.
     */
    static std::unique_ptr<Platform> CreateStandardPlatform(Log *log);

    /**
     * Creates a new platform window.
     *
     * @return                          Newly created window, or `nullptr` on error.
     */
    virtual std::unique_ptr<PlatformWindow> CreateWindow() = 0;

    /**
     * Creates a new event loop.
     *
     * @return                          Newly created event loop, or `nullptr` on error.
     */
    virtual std::unique_ptr<PlatformEventLoop> CreateEventLoop() = 0;

    /**
     * Shows / hides system cursor (on top of all windows created by this platform).
     *
     * @param cursorShown               Whether to show the system cursor.
     */
    virtual void SetCursorShown(bool cursorShown) = 0;

    /**
     * Getter for `SetCursorShown`.
     *
     * @return                          Whether the system cursor is currently shown on top of windows created by this
     *                                  platform.
     */
    virtual bool IsCursorShown() const = 0;

    /**
     * @return                          Geometries of all monitors on current system, or an empty vector in case of an
     *                                  error.
     */
    virtual std::vector<Recti> DisplayGeometries() const = 0;
};
