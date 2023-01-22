#pragma once

#include "Utility/Geometry/Point.h"
#include "Utility/Geometry/Size.h"

#include "PlatformEnums.h"

#ifdef KeyPress
#   undef KeyPress
#endif
#ifdef KeyRelease
#   undef KeyRelease
#endif

class PlatformWindow;

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
        NativeEvent,

        FirstEventType = KeyPress,
        LastEventType = WindowCloseRequest
    };
    using enum Type;

    virtual ~PlatformEvent() = default;

    Type type = Invalid;
};

class PlatformWindowEvent: public PlatformEvent {
 public:
    PlatformWindow *window = nullptr;
};

/**
 * `KeyPress` or `KeyRelease` event.
 */
class PlatformKeyEvent: public PlatformWindowEvent {
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
class PlatformMouseEvent: public PlatformWindowEvent {
 public:
    PlatformMouseButton button; // Button that caused this event, or PlatformMouseButton::None for move events.
    PlatformMouseButtons buttons; // Currently pressed mouse buttons.
    Pointi pos; // Window-relative cursor position.
    bool isDoubleClick = false;
};

/**
 * `MouseWheel`
 */
class PlatformWheelEvent: public PlatformWindowEvent {
 public:
    Pointi angleDelta; // 1 unit = 1/8 degree.
    bool inverted; // Whether delta values delivered with the event are inverted.
};

/**
 * `WindowMove`
 */
class PlatformMoveEvent: public PlatformWindowEvent {
 public:
    Pointi pos; // New position of the window.
};

/**
 * `WindowResize`
 */
class PlatformResizeEvent: public PlatformWindowEvent {
 public:
    Sizei size; // New size of the window.
};

class PlatformGamepadDeviceEvent: public PlatformEvent {
 public:
    uint32_t id;
};

/**
 * `NativeEvent`, sent only when platform is built with `MM_PLATFORM_SEND_NATIVE_EVENTS` defined.
 */
class PlatformNativeEvent: public PlatformEvent {
 public:
    const void *nativeEvent = nullptr; // Pointer to a native event, in our case this is `SDL_Event`. Never `nullptr`.
};
