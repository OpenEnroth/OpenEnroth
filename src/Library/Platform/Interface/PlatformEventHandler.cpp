#include "PlatformEventHandler.h"

void PlatformEventHandler::event(const PlatformEvent *event) {
    switch (event->type) {
    case EVENT_KEY_PRESS:
        keyPressEvent(static_cast<const PlatformKeyEvent *>(event));
        return;
    case EVENT_KEY_RELEASE:
        keyReleaseEvent(static_cast<const PlatformKeyEvent *>(event));
        return;
    case EVENT_MOUSE_MOVE:
        mouseMoveEvent(static_cast<const PlatformMouseEvent *>(event));
        return;
    case EVENT_MOUSE_BUTTON_PRESS:
        mousePressEvent(static_cast<const PlatformMouseEvent *>(event));
        return;
    case EVENT_MOUSE_BUTTON_RELEASE:
        mouseReleaseEvent(static_cast<const PlatformMouseEvent *>(event));
        return;
    case EVENT_MOUSE_WHEEL:
        wheelEvent(static_cast<const PlatformWheelEvent *>(event));
        return;
    case EVENT_WINDOW_MOVE:
        moveEvent(static_cast<const PlatformMoveEvent *>(event));
        return;
    case EVENT_WINDOW_RESIZE:
        resizeEvent(static_cast<const PlatformResizeEvent *>(event));
        return;
    case EVENT_WINDOW_ACTIVATE:
    case EVENT_WINDOW_DEACTIVATE:
        activationEvent(static_cast<const PlatformWindowEvent *>(event));
        return;
    case EVENT_WINDOW_CLOSE_REQUEST:
        closeEvent(static_cast<const PlatformWindowEvent *>(event));
        return;
    case EVENT_GAMEPAD_CONNECTED:
    case EVENT_GAMEPAD_DISCONNECTED:
        gamepadConnectionEvent(static_cast<const PlatformGamepadEvent *>(event));
        return;
    case EVENT_GAMEPAD_KEY_PRESS:
        gamepadKeyPressEvent(static_cast<const PlatformGamepadKeyEvent *>(event));
        return;
    case EVENT_GAMEPAD_KEY_RELEASE:
        gamepadKeyReleaseEvent(static_cast<const PlatformGamepadKeyEvent *>(event));
        return;
    case EVENT_GAMEPAD_AXIS:
        gamepadAxisEvent(static_cast<const PlatformGamepadAxisEvent *>(event));
        return;
    case EVENT_NATIVE:
        nativeEvent(static_cast<const PlatformNativeEvent *>(event));
        return;
    default:
        return;
    }
}

void PlatformEventHandler::keyPressEvent(const PlatformKeyEvent *) {}
void PlatformEventHandler::keyReleaseEvent(const PlatformKeyEvent *) {}
void PlatformEventHandler::mouseMoveEvent(const PlatformMouseEvent *) {}
void PlatformEventHandler::mousePressEvent(const PlatformMouseEvent *) {}
void PlatformEventHandler::mouseReleaseEvent(const PlatformMouseEvent *) {}
void PlatformEventHandler::wheelEvent(const PlatformWheelEvent *) {}
void PlatformEventHandler::moveEvent(const PlatformMoveEvent *) {}
void PlatformEventHandler::resizeEvent(const PlatformResizeEvent *) {}
void PlatformEventHandler::activationEvent(const PlatformWindowEvent *) {}
void PlatformEventHandler::closeEvent(const PlatformWindowEvent *) {}
void PlatformEventHandler::gamepadConnectionEvent(const PlatformGamepadEvent *) {}
void PlatformEventHandler::gamepadKeyPressEvent(const PlatformGamepadKeyEvent *) {}
void PlatformEventHandler::gamepadKeyReleaseEvent(const PlatformGamepadKeyEvent *) {}
void PlatformEventHandler::gamepadAxisEvent(const PlatformGamepadAxisEvent *) {}
void PlatformEventHandler::nativeEvent(const PlatformNativeEvent *) {}
