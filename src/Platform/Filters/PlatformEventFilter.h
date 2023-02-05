#pragma once

#include <vector>

#include "Platform/PlatformEvents.h"

class PlatformEventFilter {
 public:
    enum class EventWildcard { // TODO(captainurist): revisit after renaming event types for the new codestyle
        ALL_EVENTS
    };
    using enum EventWildcard;

    explicit PlatformEventFilter(std::initializer_list<PlatformEventType> eventTypes);
    explicit PlatformEventFilter(EventWildcard eventTypes);

    virtual bool event(const PlatformEvent *event);

    const std::vector<PlatformEventType> eventTypes() const {
        return _eventTypes;
    }

 protected:
    virtual bool keyPressEvent(const PlatformKeyEvent *event);
    virtual bool keyReleaseEvent(const PlatformKeyEvent *event);
    virtual bool mouseMoveEvent(const PlatformMouseEvent *event);
    virtual bool mousePressEvent(const PlatformMouseEvent *event);
    virtual bool mouseReleaseEvent(const PlatformMouseEvent *event);
    virtual bool wheelEvent(const PlatformWheelEvent *event);
    virtual bool moveEvent(const PlatformMoveEvent *event);
    virtual bool resizeEvent(const PlatformResizeEvent *event);
    virtual bool activationEvent(const PlatformWindowEvent *event);
    virtual bool closeEvent(const PlatformWindowEvent *event);
    virtual bool gamepadDeviceEvent(const PlatformGamepadDeviceEvent *event);
    virtual bool nativeEvent(const PlatformNativeEvent *event);

 private:
    std::vector<PlatformEventType> _eventTypes;
};

