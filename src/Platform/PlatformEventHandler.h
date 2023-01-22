#pragma once

#include "PlatformEvents.h"

class PlatformWindow;

/**
 * Event handler interface, to be implemented in user code.
 *
 * @see PlatformEventLoop
 */
class PlatformEventHandler {
 public:
    virtual ~PlatformEventHandler() = default;

    // TODO(captainurist): maybe PlatformWindow * belongs inside Event after all? Not all events even have windows.
    /**
     * @param window                    Platform window that generated the event.
     * @param event                     Event.
     */
    virtual void Event(PlatformWindow *window, const PlatformEvent *event);

 protected:
    virtual void KeyPressEvent(PlatformWindow *window, const PlatformKeyEvent *event);
    virtual void KeyReleaseEvent(PlatformWindow *window, const PlatformKeyEvent *event);
    virtual void MouseMoveEvent(PlatformWindow *window, const PlatformMouseEvent *event);
    virtual void MousePressEvent(PlatformWindow *window, const PlatformMouseEvent *event);
    virtual void MouseReleaseEvent(PlatformWindow *window, const PlatformMouseEvent *event);
    virtual void WheelEvent(PlatformWindow *window, const PlatformWheelEvent *event);
    virtual void MoveEvent(PlatformWindow *window, const PlatformMoveEvent *event);
    virtual void ResizeEvent(PlatformWindow *window, const PlatformResizeEvent *event);
    virtual void ActivationEvent(PlatformWindow *window, const PlatformEvent *event);
    virtual void CloseEvent(PlatformWindow *window, const PlatformEvent *event);
    virtual void GamepadDeviceEvent(PlatformWindow *window, const PlatformGamepadDeviceEvent *event);
    virtual void NativeEvent(PlatformWindow *window, const PlatformNativeEvent *event);
};
