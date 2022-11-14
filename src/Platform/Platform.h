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
     * Creates a new platform window with the provided event handler. Window takes ownership of the provided event
     * handler.
     *
     * @param eventHandler              Event handler to use.
     * @return                          Newly created window, or `nullptr` on error, in which case provided event
     *                                  handler is destroyed.
     */
    virtual std::unique_ptr<PlatformWindow> CreateWindow(std::unique_ptr<PlatformEventHandler> eventHandler) = 0;

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

    /**
     * Sends an event to the provided window. It will then be handled by the window's event handler.
     *
     * @param window                    Window to send event to.
     * @param event                     Event pointer. Note that this function does not take ownership of the provided
     *                                  event, so it is safe to allocate it on the stack.
     * @return                          Whatever the window's event handler has returned.
     */
    virtual bool SendEvent(PlatformWindow *window, PlatformEvent *event) = 0;
};
