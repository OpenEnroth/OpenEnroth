#pragma once

#include "Utility/Geometry/Point.h"
#include "Utility/Geometry/Size.h"

#include "PlatformKey.h"
#include "PlatformModifiers.h"
#include "PlatformMouseButton.h"
#include "PlatformGamepad.h"

#ifdef KeyPress
#   undef KeyPress
#endif
#ifdef KeyRelease
#   undef KeyRelease
#endif

class PlatformEvent {
 public:
    enum class Type {
        Invalid = -1,
        KeyPress = 0,
        KeyRelease,
        GamepadConnected,
        GamepadDisconnected,
        MouseButtonPress,
        MouseButtonRelease,
        MouseMove,
        MouseWheel,
        WindowMove,
        WindowResize,
        WindowActivate,
        WindowDeactivate,
        WindowCloseRequest,
        // TODO(captainurist): NativeEvent // untranslated native event, useful for prototyping.

        FirstEventType = KeyPress,
        LastEventType = WindowCloseRequest
    };
    using enum Type;

    virtual ~PlatformEvent() = default;

    Type type = Invalid;
};

/**
 * `KeyPress` or `KeyRelease` event.
 */
class PlatformKeyEvent: public PlatformEvent {
 public:
    uint32_t id; // TODO(captainurist): move into a separate PlatformGamepadEvent
    PlatformKey key;
    PlatformKeyType keyType;
    PlatformKeyValue keyValue;
    PlatformModifiers mods;
    bool isAutoRepeat = false;
};

/**
 * `MouseButtonPress`, `MouseButtonRelease` or `MouseMove` event.
 */
class PlatformMouseEvent: public PlatformEvent {
 public:
    PlatformMouseButton button; // Button that caused this event, or PlatformMouseButton::None for move events.
    PlatformMouseButtons buttons; // Currently pressed mouse buttons.
    Pointi pos; // Window-relative cursor position.
    bool isDoubleClick = false;
};

/**
 * `MouseWheel`
 */
class PlatformWheelEvent: public PlatformEvent {
 public:
    Pointi angleDelta; // 1 unit = 1/8 degree.
    bool inverted; // Whether delta values delivered with the event are inverted.
};

/**
 * `WindowMove`
 */
class PlatformMoveEvent: public PlatformEvent {
 public:
    Pointi pos; // New position of the window.
};

/**
 * `WindowResize`
 */
class PlatformResizeEvent: public PlatformEvent {
 public:
    Sizei size; // New size of the window.
};

class PlatformGamepadDeviceEvent: public PlatformEvent {
 public:
    uint32_t id;
};

// TODO(captainurist): PlatformNativeEvent { const void *nativeEvent = nullptr; }
