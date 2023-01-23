#include "PlatformEventFilter.h"

#include <cassert>

#include "Utility/Segment.h"

PlatformEventFilter::PlatformEventFilter(std::initializer_list<PlatformEvent::Type> eventTypes) : _eventTypes(eventTypes) {}

PlatformEventFilter::PlatformEventFilter(EventWildcard eventTypes) {
    assert(eventTypes == ALL_EVENTS);

    for (PlatformEvent::Type type : Segment(PlatformEvent::FirstEventType, PlatformEvent::LastEventType))
        _eventTypes.push_back(type);
}

bool PlatformEventFilter::Event(PlatformWindow *window, const PlatformEvent *event) {
    switch (event->type) {
    case PlatformEvent::GamepadConnected:
    case PlatformEvent::GamepadDisconnected:
        return GamepadDeviceEvent(window, static_cast<const PlatformGamepadDeviceEvent *>(event));
    case PlatformEvent::KeyPress:
        return KeyPressEvent(window, static_cast<const PlatformKeyEvent *>(event));
    case PlatformEvent::KeyRelease:
        return KeyReleaseEvent(window, static_cast<const PlatformKeyEvent *>(event));
    case PlatformEvent::MouseMove:
        return MouseMoveEvent(window, static_cast<const PlatformMouseEvent *>(event));
    case PlatformEvent::MouseButtonPress:
        return MousePressEvent(window, static_cast<const PlatformMouseEvent *>(event));
    case PlatformEvent::MouseButtonRelease:
        return MouseReleaseEvent(window, static_cast<const PlatformMouseEvent *>(event));
    case PlatformEvent::MouseWheel:
        return WheelEvent(window, static_cast<const PlatformWheelEvent *>(event));
    case PlatformEvent::WindowMove:
        return MoveEvent(window, static_cast<const PlatformMoveEvent *>(event));
    case PlatformEvent::WindowResize:
        return ResizeEvent(window, static_cast<const PlatformResizeEvent *>(event));
    case PlatformEvent::WindowActivate:
    case PlatformEvent::WindowDeactivate:
        return ActivationEvent(window, event);
    case PlatformEvent::WindowCloseRequest:
        return CloseEvent(window, event);
    default:
        return false;
    }
}

bool PlatformEventFilter::KeyPressEvent(PlatformWindow *, const PlatformKeyEvent *) {
    return false;
}

bool PlatformEventFilter::KeyReleaseEvent(PlatformWindow *, const PlatformKeyEvent *) {
    return false;
}

bool PlatformEventFilter::MouseMoveEvent(PlatformWindow *, const PlatformMouseEvent *) {
    return false;
}

bool PlatformEventFilter::MousePressEvent(PlatformWindow *, const PlatformMouseEvent *) {
    return false;
}

bool PlatformEventFilter::MouseReleaseEvent(PlatformWindow *, const PlatformMouseEvent *) {
    return false;
}

bool PlatformEventFilter::WheelEvent(PlatformWindow *, const PlatformWheelEvent *) {
    return false;
}

bool PlatformEventFilter::MoveEvent(PlatformWindow *, const PlatformMoveEvent *) {
    return false;
}

bool PlatformEventFilter::ResizeEvent(PlatformWindow *, const PlatformResizeEvent *) {
    return false;
}

bool PlatformEventFilter::ActivationEvent(PlatformWindow *, const PlatformEvent *) {
    return false;
}

bool PlatformEventFilter::CloseEvent(PlatformWindow *, const PlatformEvent *) {
    return false;
}

bool PlatformEventFilter::GamepadDeviceEvent(PlatformWindow *, const PlatformGamepadDeviceEvent *) {
    return false;
}

