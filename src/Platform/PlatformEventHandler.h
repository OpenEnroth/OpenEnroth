#pragma once

#include "PlatformEvents.h"

class PlatformEventHandler {
 public:
    virtual ~PlatformEventHandler() = default;

    virtual bool Event(const PlatformEvent *event);

 protected:
    virtual bool KeyPressEvent(const PlatformKeyEvent *event);
    virtual bool KeyReleaseEvent(const PlatformKeyEvent *event);
    virtual bool MouseMoveEvent(const PlatformMouseEvent *event);
    virtual bool MousePressEvent(const PlatformMouseEvent *event);
    virtual bool MouseReleaseEvent(const PlatformMouseEvent *event);
    virtual bool WheelEvent(const PlatformWheelEvent *event);
    virtual bool ActivationEvent(const PlatformEvent *event);
};
