#pragma once

#include <Library/Platform/Interface/PlatformEvents.h>

class FSMEventHandler {
 public:
    virtual bool keyPressEvent(const PlatformKeyEvent *event) { return false; }
    virtual bool keyReleaseEvent(const PlatformKeyEvent *event) { return false; }
    virtual bool mouseMoveEvent(const PlatformMouseEvent *event) { return false; }
    virtual bool mousePressEvent(const PlatformMouseEvent *event) { return false; }
    virtual bool mouseReleaseEvent(const PlatformMouseEvent *event) { return false; }
    virtual bool wheelEvent(const PlatformWheelEvent *event) { return false; }
    virtual bool moveEvent(const PlatformMoveEvent *event) { return false; }
    virtual bool resizeEvent(const PlatformResizeEvent *event) { return false; }
    virtual bool activationEvent(const PlatformWindowEvent *event) { return false; }
    virtual bool closeEvent(const PlatformWindowEvent *event) { return false; }
    virtual bool gamepadConnectionEvent(const PlatformGamepadEvent *event) { return false; }
    virtual bool gamepadKeyPressEvent(const PlatformGamepadKeyEvent *event) { return false; }
    virtual bool gamepadKeyReleaseEvent(const PlatformGamepadKeyEvent *event) { return false; }
    virtual bool gamepadAxisEvent(const PlatformGamepadAxisEvent *event) { return false; }
    virtual bool nativeEvent(const PlatformNativeEvent *event) { return false; }
    virtual bool textInputEvent(const PlatformTextInputEvent *event) { return false; }
};
