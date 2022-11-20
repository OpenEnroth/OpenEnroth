#pragma once

#include "Utility/Geometry/Point.h"

#include "PlatformKey.h"
#include "PlatformModifiers.h"
#include "PlatformMouseButton.h"

#ifdef KeyPress
#   undef KeyPress
#endif
#ifdef KeyRelease
#   undef KeyRelease
#endif

class PlatformEvent {
 public:
    enum class Type {
        Invalid,
        KeyPress,
        KeyRelease,
        MouseButtonPress,
        MouseButtonRelease,
        MouseMove,
        MouseWheel,
        WindowMove,
        WindowActivate,
        WindowDeactivate,
        WindowCloseRequest,
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
    PlatformKey key;
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
