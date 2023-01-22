#include "PlatformEventHandler.h"

void PlatformEventHandler::Event(const PlatformEvent *event) {
    switch (event->type) {
    case PlatformEvent::GamepadConnected:
    case PlatformEvent::GamepadDisconnected:
        GamepadDeviceEvent(static_cast<const PlatformGamepadDeviceEvent *>(event));
        return;
    case PlatformEvent::KeyPress:
        KeyPressEvent(static_cast<const PlatformKeyEvent *>(event));
        return;
    case PlatformEvent::KeyRelease:
        KeyReleaseEvent(static_cast<const PlatformKeyEvent *>(event));
        return;
    case PlatformEvent::MouseMove:
        MouseMoveEvent(static_cast<const PlatformMouseEvent *>(event));
        return;
    case PlatformEvent::MouseButtonPress:
        MousePressEvent(static_cast<const PlatformMouseEvent *>(event));
        return;
    case PlatformEvent::MouseButtonRelease:
        MouseReleaseEvent(static_cast<const PlatformMouseEvent *>(event));
        return;
    case PlatformEvent::MouseWheel:
        WheelEvent(static_cast<const PlatformWheelEvent *>(event));
        return;
    case PlatformEvent::WindowMove:
        MoveEvent(static_cast<const PlatformMoveEvent *>(event));
        return;
    case PlatformEvent::WindowResize:
        ResizeEvent(static_cast<const PlatformResizeEvent *>(event));
        return;
    case PlatformEvent::WindowActivate:
    case PlatformEvent::WindowDeactivate:
        ActivationEvent(static_cast<const PlatformWindowEvent *>(event));
        return;
    case PlatformEvent::WindowCloseRequest:
        CloseEvent(static_cast<const PlatformWindowEvent *>(event));
        return;
    case PlatformEvent::NativeEvent:
        NativeEvent(static_cast<const PlatformNativeEvent *>(event));
        return;
    default:
        return;
    }
}

void PlatformEventHandler::KeyPressEvent(const PlatformKeyEvent *) {}
void PlatformEventHandler::KeyReleaseEvent(const PlatformKeyEvent *) {}
void PlatformEventHandler::MouseMoveEvent(const PlatformMouseEvent *) {}
void PlatformEventHandler::MousePressEvent(const PlatformMouseEvent *) {}
void PlatformEventHandler::MouseReleaseEvent(const PlatformMouseEvent *) {}
void PlatformEventHandler::WheelEvent(const PlatformWheelEvent *) {}
void PlatformEventHandler::MoveEvent(const PlatformMoveEvent *) {}
void PlatformEventHandler::ResizeEvent(const PlatformResizeEvent *) {}
void PlatformEventHandler::ActivationEvent(const PlatformWindowEvent *) {}
void PlatformEventHandler::CloseEvent(const PlatformWindowEvent *) {}
void PlatformEventHandler::GamepadDeviceEvent(const PlatformGamepadDeviceEvent *) {}
void PlatformEventHandler::NativeEvent(const PlatformNativeEvent *) {}
