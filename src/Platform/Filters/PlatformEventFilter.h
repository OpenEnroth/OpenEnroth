#pragma once

#include <vector>

#include "Platform/PlatformEvents.h"

class PlatformWindow;

class PlatformEventFilter {
 public:
    enum class EventWildcard { // TODO(captainurist): revisit after renaming event types for the new codestyle
        ALL_EVENTS
    };
    using enum EventWildcard;

    explicit PlatformEventFilter(std::initializer_list<PlatformEvent::Type> eventTypes);
    explicit PlatformEventFilter(EventWildcard eventTypes);

    virtual bool Event(PlatformWindow *window, const PlatformEvent *event);

    const std::vector<PlatformEvent::Type> eventTypes() const {
        return _eventTypes;
    }

 protected:
    virtual bool KeyPressEvent(PlatformWindow *window, const PlatformKeyEvent *event);
    virtual bool KeyReleaseEvent(PlatformWindow *window, const PlatformKeyEvent *event);
    virtual bool MouseMoveEvent(PlatformWindow *window, const PlatformMouseEvent *event);
    virtual bool MousePressEvent(PlatformWindow *window, const PlatformMouseEvent *event);
    virtual bool MouseReleaseEvent(PlatformWindow *window, const PlatformMouseEvent *event);
    virtual bool WheelEvent(PlatformWindow *window, const PlatformWheelEvent *event);
    virtual bool MoveEvent(PlatformWindow *window, const PlatformMoveEvent *event);
    virtual bool ResizeEvent(PlatformWindow *window, const PlatformResizeEvent *event);
    virtual bool ActivationEvent(PlatformWindow *window, const PlatformEvent *event);
    virtual bool CloseEvent(PlatformWindow *window, const PlatformEvent *event);
    virtual bool GamepadDeviceEvent(PlatformWindow *window, const PlatformGamepadDeviceEvent *event);
    virtual bool NativeEvent(PlatformWindow *window, const PlatformNativeEvent *event);

 private:
    std::vector<PlatformEvent::Type> _eventTypes;
};

