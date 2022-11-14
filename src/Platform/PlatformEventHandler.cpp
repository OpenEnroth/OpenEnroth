#include "PlatformEventHandler.h"


bool PlatformEventHandler::Event(const PlatformEvent *event) {
    switch (event->type) {
    case PlatformEvent::KeyPress:
        return KeyPressEvent(static_cast<const PlatformKeyEvent *>(event));
    case PlatformEvent::KeyRelease:
        return KeyReleaseEvent(static_cast<const PlatformKeyEvent *>(event));
    case PlatformEvent::MouseMove:
        return MouseMoveEvent(static_cast<const PlatformMouseEvent *>(event));
    case PlatformEvent::MouseButtonPress:
        return MousePressEvent(static_cast<const PlatformMouseEvent *>(event));
    case PlatformEvent::MouseButtonRelease:
        return MouseReleaseEvent(static_cast<const PlatformMouseEvent *>(event));
    case PlatformEvent::MouseWheel:
        return WheelEvent(static_cast<const PlatformWheelEvent *>(event));
    case PlatformEvent::WindowActivated:
    case PlatformEvent::WindowDeactivated:
        return ActivationEvent(event);
    default:
        return false;
    }
}

bool PlatformEventHandler::KeyPressEvent(const PlatformKeyEvent *) {
    return false;
}

bool PlatformEventHandler::KeyReleaseEvent(const PlatformKeyEvent *) {
    return false;
}

bool PlatformEventHandler::MouseMoveEvent(const PlatformMouseEvent *) {
    return false;
}

bool PlatformEventHandler::MousePressEvent(const PlatformMouseEvent *) {
    return false;
}

bool PlatformEventHandler::MouseReleaseEvent(const PlatformMouseEvent *) {
    return false;
}

bool PlatformEventHandler::WheelEvent(const PlatformWheelEvent *event) {
    return false;
}

bool PlatformEventHandler::ActivationEvent(const PlatformEvent *event) {
    return false;
}
