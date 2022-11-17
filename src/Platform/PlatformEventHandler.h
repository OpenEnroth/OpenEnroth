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
    virtual void ActivationEvent(PlatformWindow *window, const PlatformEvent *event);
    virtual void CloseEvent(PlatformWindow *window, const PlatformEvent *event);
};
