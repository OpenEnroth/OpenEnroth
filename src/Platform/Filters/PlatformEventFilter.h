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

    virtual bool Event(const PlatformEvent *event);

    const std::vector<PlatformEvent::Type> eventTypes() const {
        return _eventTypes;
    }

 protected:
    virtual bool KeyPressEvent(const PlatformKeyEvent *event);
    virtual bool KeyReleaseEvent(const PlatformKeyEvent *event);
    virtual bool MouseMoveEvent(const PlatformMouseEvent *event);
    virtual bool MousePressEvent(const PlatformMouseEvent *event);
    virtual bool MouseReleaseEvent(const PlatformMouseEvent *event);
    virtual bool WheelEvent(const PlatformWheelEvent *event);
    virtual bool MoveEvent(const PlatformMoveEvent *event);
    virtual bool ResizeEvent(const PlatformResizeEvent *event);
    virtual bool ActivationEvent(const PlatformWindowEvent *event);
    virtual bool CloseEvent(const PlatformWindowEvent *event);
    virtual bool GamepadDeviceEvent(const PlatformGamepadDeviceEvent *event);
    virtual bool NativeEvent(const PlatformNativeEvent *event);

 private:
    std::vector<PlatformEvent::Type> _eventTypes;
};

