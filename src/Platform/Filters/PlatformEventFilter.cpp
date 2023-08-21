#include "PlatformEventFilter.h"

#include <cassert>

#include "Utility/Segment.h"
#include "Platform/PlatformEnums.h"
#include "Platform/PlatformEvents.h"

PlatformEventFilter::PlatformEventFilter(std::initializer_list<PlatformEventType> eventTypes) : _eventTypes(eventTypes) {}

PlatformEventFilter::PlatformEventFilter(PlatformEventWildcard eventTypes) {
    assert(eventTypes == EVENTS_ALL);

    for (PlatformEventType type : Segment(EVENT_FIRST, EVENT_LAST))
        _eventTypes.push_back(type);
}

bool PlatformEventFilter::event(const PlatformEvent *event) {
    switch (event->type) {
    case EVENT_KEY_PRESS:
        return keyPressEvent(static_cast<const PlatformKeyEvent *>(event));
    case EVENT_KEY_RELEASE:
        return keyReleaseEvent(static_cast<const PlatformKeyEvent *>(event));
    case EVENT_MOUSE_MOVE:
        return mouseMoveEvent(static_cast<const PlatformMouseEvent *>(event));
    case EVENT_MOUSE_BUTTON_PRESS:
        return mousePressEvent(static_cast<const PlatformMouseEvent *>(event));
    case EVENT_MOUSE_BUTTON_RELEASE:
        return mouseReleaseEvent(static_cast<const PlatformMouseEvent *>(event));
    case EVENT_MOUSE_WHEEL:
        return wheelEvent(static_cast<const PlatformWheelEvent *>(event));
    case EVENT_WINDOW_MOVE:
        return moveEvent(static_cast<const PlatformMoveEvent *>(event));
    case EVENT_WINDOW_RESIZE:
        return resizeEvent(static_cast<const PlatformResizeEvent *>(event));
    case EVENT_WINDOW_ACTIVATE:
    case EVENT_WINDOW_DEACTIVATE:
        return activationEvent(static_cast<const PlatformWindowEvent *>(event));
    case EVENT_WINDOW_CLOSE_REQUEST:
        return closeEvent(static_cast<const PlatformWindowEvent *>(event));
    case EVENT_GAMEPAD_CONNECTED:
    case EVENT_GAMEPAD_DISCONNECTED:
        return gamepadConnectionEvent(static_cast<const PlatformGamepadEvent *>(event));
    case EVENT_GAMEPAD_KEY_PRESS:
        return gamepadKeyPressEvent(static_cast<const PlatformGamepadKeyEvent *>(event));
    case EVENT_GAMEPAD_KEY_RELEASE:
        return gamepadKeyReleaseEvent(static_cast<const PlatformGamepadKeyEvent *>(event));
    case EVENT_GAMEPAD_AXIS:
        return gamepadAxisEvent(static_cast<const PlatformGamepadAxisEvent *>(event));
    case EVENT_NATIVE:
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

bool PlatformEventFilter::gamepadConnectionEvent(const PlatformGamepadEvent *) {
    return false;
}

bool PlatformEventFilter::gamepadKeyPressEvent(const PlatformGamepadKeyEvent *) {
    return false;
}

bool PlatformEventFilter::gamepadKeyReleaseEvent(const PlatformGamepadKeyEvent *) {
    return false;
}

bool PlatformEventFilter::gamepadAxisEvent(const PlatformGamepadAxisEvent *) {
    return false;
}

bool PlatformEventFilter::nativeEvent(const PlatformNativeEvent *) {
    return false;
}
