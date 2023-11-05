#pragma once

#include <cstdint>

#include "Utility/Geometry/Point.h"
#include "Utility/Geometry/Size.h"

#include "PlatformEnums.h"

class PlatformWindow;
class PlatformGamepad;

class PlatformEvent {
 public:
    virtual ~PlatformEvent() = default;

    PlatformEventType type = EVENT_INVALID;
};

class PlatformWindowEvent: public PlatformEvent {
 public:
    PlatformWindow *window = nullptr;
};

class PlatformKeyEvent: public PlatformWindowEvent {
 public:
    PlatformKey key;
    PlatformModifiers mods;
    bool isAutoRepeat = false;
};

class PlatformMouseEvent: public PlatformWindowEvent {
 public:
    PlatformMouseButton button; // Button that caused this event, or BUTTON_NONE for move events.
    PlatformMouseButtons buttons; // Currently pressed mouse buttons.
    Pointi pos; // Window-relative cursor position.
    bool isDoubleClick = false;
};

class PlatformWheelEvent: public PlatformWindowEvent {
 public:
    Pointi angleDelta; // 1 unit = 1/8 degree.
};

class PlatformMoveEvent: public PlatformWindowEvent {
 public:
    Pointi pos; // New position of the window.
};

class PlatformResizeEvent: public PlatformWindowEvent {
 public:
    Sizei size; // New size of the window.
};

class PlatformGamepadEvent: public PlatformEvent {
 public:
    PlatformGamepad *gamepad = nullptr;
};

class PlatformGamepadKeyEvent: public PlatformGamepadEvent {
 public:
    PlatformKey key; // TODO(captainurist): PlatformGamepadKey
};

class PlatformGamepadAxisEvent: public PlatformGamepadEvent {
 public:
    PlatformKey axis; // TODO(captainurist): PlatformGamepadAxis
    float value = 0.0; // In [-1, 1] range.
};

class PlatformNativeEvent: public PlatformEvent {
 public:
    const void *nativeEvent = nullptr; // Pointer to a native event, in our case this is `SDL_Event`. Never `nullptr`.
};
