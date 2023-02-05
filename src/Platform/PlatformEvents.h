#pragma once

#include <cstdint>

#include "Utility/Geometry/Point.h"
#include "Utility/Geometry/Size.h"

#include "PlatformEnums.h"

#undef KeyPress
#undef KeyRelease

class PlatformWindow;

class PlatformEvent {
 public:
    virtual ~PlatformEvent() = default;

    PlatformEventType type = EVENT_INVALID;
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
    PlatformMouseButton button; // Button that caused this event, or BUTTON_NONE for move events.
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
