#include "PlatformEventHandler.h"

void PlatformEventHandler::event(const PlatformEvent *event) {
    switch (event->type) {
    case PlatformEvent::GamepadConnected:
    case PlatformEvent::GamepadDisconnected:
        gamepadDeviceEvent(static_cast<const PlatformGamepadDeviceEvent *>(event));
        return;
    case PlatformEvent::KeyPress:
        keyPressEvent(static_cast<const PlatformKeyEvent *>(event));
        return;
    case PlatformEvent::KeyRelease:
        keyReleaseEvent(static_cast<const PlatformKeyEvent *>(event));
        return;
    case PlatformEvent::MouseMove:
        mouseMoveEvent(static_cast<const PlatformMouseEvent *>(event));
        return;
    case PlatformEvent::MouseButtonPress:
        mousePressEvent(static_cast<const PlatformMouseEvent *>(event));
        return;
    case PlatformEvent::MouseButtonRelease:
        mouseReleaseEvent(static_cast<const PlatformMouseEvent *>(event));
        return;
    case PlatformEvent::MouseWheel:
        wheelEvent(static_cast<const PlatformWheelEvent *>(event));
        return;
    case PlatformEvent::WindowMove:
        moveEvent(static_cast<const PlatformMoveEvent *>(event));
        return;
    case PlatformEvent::WindowResize:
        resizeEvent(static_cast<const PlatformResizeEvent *>(event));
        return;
    case PlatformEvent::WindowActivate:
    case PlatformEvent::WindowDeactivate:
        activationEvent(static_cast<const PlatformWindowEvent *>(event));
        return;
    case PlatformEvent::WindowCloseRequest:
        closeEvent(static_cast<const PlatformWindowEvent *>(event));
        return;
    case PlatformEvent::NativeEvent:
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
void PlatformEventHandler::gamepadDeviceEvent(const PlatformGamepadDeviceEvent *) {}
void PlatformEventHandler::nativeEvent(const PlatformNativeEvent *) {}
