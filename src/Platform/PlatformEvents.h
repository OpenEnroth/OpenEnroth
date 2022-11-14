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
        WindowActivated,
        WindowDeactivated,
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
    bool isAutoRepeat = false;
    PlatformKey key;
    PlatformModifiers mods;
};

/**
 * `MouseButtonPress`, `MouseButtonRelease` or `MouseMove` event.
 */
class PlatformMouseEvent: public PlatformEvent {
 public:
    PlatformMouseButton button; // Button that caused this event, or PlatformMouseButton::None for move events.
    PlatformMouseButtons buttons; // Currently pressed mouse buttons.
    bool isDoubleClick = false;
    Pointi pos; // Window-relative cursor position.
};

/**
 * `MouseWheel`
 */
class PlatformWheelEvent: public PlatformEvent {
 public:
    bool inverted; // Whether delta values delivered with the event are inverted.
    Pointi angleDelta; // 1 unit = 1/8 degree.
};
