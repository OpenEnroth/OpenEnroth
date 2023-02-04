#include "PlatformEventFilter.h"

#include <cassert>

#include "Utility/Segment.h"

PlatformEventFilter::PlatformEventFilter(std::initializer_list<PlatformEvent::Type> eventTypes) : _eventTypes(eventTypes) {}

PlatformEventFilter::PlatformEventFilter(EventWildcard eventTypes) {
    assert(eventTypes == ALL_EVENTS);

    for (PlatformEvent::Type type : Segment(PlatformEvent::FirstEventType, PlatformEvent::LastEventType))
        _eventTypes.push_back(type);
}

bool PlatformEventFilter::event(const PlatformEvent *event) {
    switch (event->type) {
    case PlatformEvent::GamepadConnected:
    case PlatformEvent::GamepadDisconnected:
        return gamepadDeviceEvent(static_cast<const PlatformGamepadDeviceEvent *>(event));
    case PlatformEvent::KeyPress:
        return keyPressEvent(static_cast<const PlatformKeyEvent *>(event));
    case PlatformEvent::KeyRelease:
        return keyReleaseEvent(static_cast<const PlatformKeyEvent *>(event));
    case PlatformEvent::MouseMove:
        return mouseMoveEvent(static_cast<const PlatformMouseEvent *>(event));
    case PlatformEvent::MouseButtonPress:
        return mousePressEvent(static_cast<const PlatformMouseEvent *>(event));
    case PlatformEvent::MouseButtonRelease:
        return mouseReleaseEvent(static_cast<const PlatformMouseEvent *>(event));
    case PlatformEvent::MouseWheel:
        return wheelEvent(static_cast<const PlatformWheelEvent *>(event));
    case PlatformEvent::WindowMove:
        return moveEvent(static_cast<const PlatformMoveEvent *>(event));
    case PlatformEvent::WindowResize:
        return resizeEvent(static_cast<const PlatformResizeEvent *>(event));
    case PlatformEvent::WindowActivate:
    case PlatformEvent::WindowDeactivate:
        return activationEvent(static_cast<const PlatformWindowEvent *>(event));
    case PlatformEvent::WindowCloseRequest:
        return closeEvent(static_cast<const PlatformWindowEvent *>(event));
    case PlatformEvent::NativeEvent:
        return nativeEvent(static_cast<const PlatformNativeEvent *>(event));
    default:
        return false;
    }
}

bool PlatformEventFilter::keyPressEvent(const PlatformKeyEvent *) {
    return false;
}

bool PlatformEventFilter::keyReleaseEvent(const PlatformKeyEvent *) {
    return false;
}

bool PlatformEventFilter::mouseMoveEvent(const PlatformMouseEvent *) {
    return false;
}

bool PlatformEventFilter::mousePressEvent(const PlatformMouseEvent *) {
    return false;
}

bool PlatformEventFilter::mouseReleaseEvent(const PlatformMouseEvent *) {
    return false;
}

bool PlatformEventFilter::wheelEvent(const PlatformWheelEvent *) {
    return false;
}

bool PlatformEventFilter::moveEvent(const PlatformMoveEvent *) {
    return false;
}

bool PlatformEventFilter::resizeEvent(const PlatformResizeEvent *) {
    return false;
}

bool PlatformEventFilter::activationEvent(const PlatformWindowEvent *) {
    return false;
}

bool PlatformEventFilter::closeEvent(const PlatformWindowEvent *) {
    return false;
}

bool PlatformEventFilter::gamepadDeviceEvent(const PlatformGamepadDeviceEvent *) {
    return false;
}

bool PlatformEventFilter::nativeEvent(const PlatformNativeEvent *) {
    return false;
}
