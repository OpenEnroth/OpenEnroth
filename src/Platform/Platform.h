#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Utility/Geometry/Rect.h"
#include "Utility/Flags.h"

#include "PlatformEnums.h"

#ifdef CreateWindow
#   undef CreateWindow
#endif

class PlatformEvent;
class PlatformWindow;
class PlatformEventLoop;
class PlatformEventHandler;
class PlatformLogger;
class PlatformGamepad;

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
 * That's it, there is only one extension point. If you want fancy event handler chains, you can
 * use `FilteringEventHandler`.
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
 *
 * Some guidelines on adding new functionality to platform classes:
 * - If something can be done at the next abstraction layer, then it *should* be done at the next abstraction layer.
 *   Don't clutter platform API.
 * - Platform API should be reasonably minimal, and methods should be orthogonal. You can think of orthogonality in
 *   terms of information content, "is this method returning bits that can also be accessed through another method?"
 *   E.g. compare `Position` + `SetPosition` + `FramePosition` + `SetFramePosition` for a window class, vs `Position` +
 *   `SetPosition` + `FrameMargins`. The latter set of methods is orthogonal, the former is not.
 * - The points above mean that platform API handles shouldn't expose non-virtual methods.
 * - Platform API should be getter/setter symmetric as long as the underlying OS API is symmetric. E.g. in the example
 *   above there is no `SetFrameMargins` method because this is not how window frames work.
 * - Platform API handles shouldn't hold state. Don't expose APIs that take ownership of user-allocated data, and don't
 *   cache values returned by the OS (if needed, caching should be implemented at the next abstraction layer). Again,
 *   you can think about this in terms of information content, "platform API handle doesn't expose any bits of state
 *   through its API that cannot be accessed through the underlying OS API".
 */
class Platform {
 public:
    virtual ~Platform() = default;

    /**
     * Creates a standard platform.
     *
     * @param logger                    Logger to use. Must not be null.
     * @return                          A newly created `Platform`. This method is guaranteed to succeed.
     */
    static std::unique_ptr<Platform> CreateStandardPlatform(PlatformLogger *logger);

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

    // TODO(captainurist): virtual PlatformGamepadInfo gamepadInfo(uint32_t id) const = 0;
    /**
     * Creates a new gamepad.
     *
     * @param id                        Gamepad identifier from event.
     * @return                          Newly created gamepad, or `nullptr` on error.
     */
    virtual std::unique_ptr<PlatformGamepad> CreateGamepad(uint32_t id) = 0;

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

    // TODO(captainurist): TBH the argument order with the title going first makes more sense
    /**
     * Shows a modal message box.
     *
     * @param message                   Message to display.
     * @param title                     Title of the message box window.
     */
    virtual void ShowMessageBox(const std::string &message, const std::string& title) const = 0;

    /**
     * @return                          Current value of a monotonic clock in milliseconds.
     */
    virtual int64_t TickCount() const = 0;

    /**
     * Windows-only function for querying the registry. Always returns an empty string on non-Windows systems.
     *
     * @param path                      Registry path to query.
     * @return                          Value at the given path, or an empty string in case of an error.
     */
    virtual std::string WinQueryRegistry(const std::wstring &path) const = 0;

    /**
     * Get various application filesystem paths.
     *
     * @param type                      Storage type.
     */
    virtual std::string StoragePath(const PLATFORM_STORAGE type) const = 0;
};
