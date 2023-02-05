#pragma once

#include <vector>

#include "Platform/PlatformEvents.h"

enum class PlatformEventWildcard {
    EVENTS_ALL
};
using enum PlatformEventWildcard;

class PlatformEventFilter {
 public:
    explicit PlatformEventFilter(std::initializer_list<PlatformEventType> eventTypes);
    explicit PlatformEventFilter(PlatformEventWildcard eventTypes);

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

