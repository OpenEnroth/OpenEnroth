#include "PlatformEventFilter.h"

#include <cassert>

#include "Utility/Segment.h"

PlatformEventFilter::PlatformEventFilter(std::initializer_list<PlatformEvent::Type> eventTypes) : _eventTypes(eventTypes) {}

PlatformEventFilter::PlatformEventFilter(EventWildcard eventTypes) {
    assert(eventTypes == ALL_EVENTS);

    for (PlatformEvent::Type type : Segment(PlatformEvent::FirstEventType, PlatformEvent::LastEventType))
        _eventTypes.push_back(type);
}

bool PlatformEventFilter::Event(const PlatformEvent *event) {
    switch (event->type) {
    case PlatformEvent::GamepadConnected:
    case PlatformEvent::GamepadDisconnected:
        return GamepadDeviceEvent(static_cast<const PlatformGamepadDeviceEvent *>(event));
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
    case PlatformEvent::WindowMove:
        return MoveEvent(static_cast<const PlatformMoveEvent *>(event));
    case PlatformEvent::WindowResize:
        return ResizeEvent(static_cast<const PlatformResizeEvent *>(event));
    case PlatformEvent::WindowActivate:
    case PlatformEvent::WindowDeactivate:
        return ActivationEvent(static_cast<const PlatformWindowEvent *>(event));
    case PlatformEvent::WindowCloseRequest:
        return CloseEvent(static_cast<const PlatformWindowEvent *>(event));
    case PlatformEvent::NativeEvent:
        return NativeEvent(static_cast<const PlatformNativeEvent *>(event));
    default:
        return false;
    }
}

bool PlatformEventFilter::KeyPressEvent(const PlatformKeyEvent *) {
    return false;
}

bool PlatformEventFilter::KeyReleaseEvent(const PlatformKeyEvent *) {
    return false;
}

bool PlatformEventFilter::MouseMoveEvent(const PlatformMouseEvent *) {
    return false;
}

bool PlatformEventFilter::MousePressEvent(const PlatformMouseEvent *) {
    return false;
}

bool PlatformEventFilter::MouseReleaseEvent(const PlatformMouseEvent *) {
    return false;
}

bool PlatformEventFilter::WheelEvent(const PlatformWheelEvent *) {
    return false;
}

bool PlatformEventFilter::MoveEvent(const PlatformMoveEvent *) {
    return false;
}

bool PlatformEventFilter::ResizeEvent(const PlatformResizeEvent *) {
    return false;
}

bool PlatformEventFilter::ActivationEvent(const PlatformWindowEvent *) {
    return false;
}

bool PlatformEventFilter::CloseEvent(const PlatformWindowEvent *) {
    return false;
}

bool PlatformEventFilter::GamepadDeviceEvent(const PlatformGamepadDeviceEvent *) {
    return false;
}

bool PlatformEventFilter::NativeEvent(const PlatformNativeEvent *) {
    return false;
}
