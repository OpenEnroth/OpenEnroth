#pragma once

#include "PlatformEvents.h"

/**
 * Event handler interface, to be implemented in user code.
 *
 * @see PlatformEventLoop
 * @see FilteringEventHandler
 */
class PlatformEventHandler {
 public:
    virtual ~PlatformEventHandler() = default;

    virtual void event(const PlatformEvent *event);

 protected:
    virtual void keyPressEvent(const PlatformKeyEvent *event);
    virtual void keyReleaseEvent(const PlatformKeyEvent *event);
    virtual void mouseMoveEvent(const PlatformMouseEvent *event);
    virtual void mousePressEvent(const PlatformMouseEvent *event);
    virtual void mouseReleaseEvent(const PlatformMouseEvent *event);
    virtual void wheelEvent(const PlatformWheelEvent *event);
    virtual void moveEvent(const PlatformMoveEvent *event);
    virtual void resizeEvent(const PlatformResizeEvent *event);
    virtual void activationEvent(const PlatformWindowEvent *event);
    virtual void closeEvent(const PlatformWindowEvent *event);
    virtual void gamepadConnectionEvent(const PlatformGamepadEvent *event);
    virtual void gamepadKeyPressEvent(const PlatformGamepadKeyEvent *event);
    virtual void gamepadKeyReleaseEvent(const PlatformGamepadKeyEvent *event);
    virtual void gamepadAxisEvent(const PlatformGamepadAxisEvent *event);
    virtual void nativeEvent(const PlatformNativeEvent *event);
};
