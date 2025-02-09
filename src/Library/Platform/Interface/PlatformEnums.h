#pragma once

#include <cstdint>

#include "Utility/Flags.h"

#undef MOD_ALT
#undef MOD_SHIFT
#undef MOD_CONTROL  // We're using MOD_CTRL, but would rather not get this more confusing.
#undef MOD_WIN      // Same here, we have MOD_META instead.

enum class PlatformEventType {
    EVENT_INVALID = -1,

    /** Key pressed, sent as `PlatformKeyEvent`. */
    EVENT_KEY_PRESS = 0,

    /** Key released, sent as `PlatformKeyEvent`. */
    EVENT_KEY_RELEASE,

    /** Mouse button pressed, sent as `PlatformMouseEvent`. */
    EVENT_MOUSE_BUTTON_PRESS,

    /** Mouse button released, sent as `PlatformMouseEvent`. */
    EVENT_MOUSE_BUTTON_RELEASE,

    /** Mouse pointer moved, sent as `PlatformMouseEvent`. */
    EVENT_MOUSE_MOVE,

    /** Mouse wheel rotated, sent as `PlatformWheelEvent`. */
    EVENT_MOUSE_WHEEL,

    /** Window moved, sent as `PlatformMoveEvent`. */
    EVENT_WINDOW_MOVE,

    /** Window resized, sent as `PlatformResizeEvent`. */
    EVENT_WINDOW_RESIZE,

    /** Window activated (gained focus), sent as `PlatformWindowEvent`. */
    EVENT_WINDOW_ACTIVATE,

    /** Window deactivated (lost focus), sent as `PlatformWindowEvent`. */
    EVENT_WINDOW_DEACTIVATE,

    /** Window close was requested (e.g. by pressing the window close button), sent as `PlatformWindowEvent`. */
    EVENT_WINDOW_CLOSE_REQUEST,

    /** Gamepad was added to the system, sent as `PlatformGamepadEvent`. */
    EVENT_GAMEPAD_CONNECTED,

    /** Gamepad was removed from the system, sent as `PlatformGamepadEvent`. */
    EVENT_GAMEPAD_DISCONNECTED,

    /** Gamepad key pressed, sent as `PlatformGamepadKeyEvent`. */
    EVENT_GAMEPAD_KEY_PRESS,

    /** Gamepad key released, sent as `PlatformGamepadKeyEvent`. */
    EVENT_GAMEPAD_KEY_RELEASE,

    /** Gamepad axis event, sent as `PlatformGamepadAxisEvent`. */
    EVENT_GAMEPAD_AXIS,

    /** Text input event, sent as `PlatformTextInputEvent`. */
    EVENT_TEXT_INPUT,

    /** Native event, sent as `PlatformNativeEvent`. */
    EVENT_NATIVE,

    EVENT_FIRST = EVENT_KEY_PRESS,
    EVENT_LAST = EVENT_NATIVE
};
using enum PlatformEventType;

enum class PlatformKey : int {
    // usual text input
    KEY_CHAR, // TODO(captainurist): this doesn't belong here

    // scancodes (language and case neutral)
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_F13,
    KEY_F14,
    KEY_F15,
    KEY_F16,
    KEY_F17,
    KEY_F18,
    KEY_F19,
    KEY_F20,
    KEY_F21,
    KEY_F22,
    KEY_F23,
    KEY_F24,

    KEY_DIGIT_0, KEY_DIGIT_1, KEY_DIGIT_2, KEY_DIGIT_3, KEY_DIGIT_4, KEY_DIGIT_5, KEY_DIGIT_6, KEY_DIGIT_7, KEY_DIGIT_8, KEY_DIGIT_9,

    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G,
    KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P,
    KEY_Q, KEY_R, KEY_S,
    KEY_T, KEY_U, KEY_V,
    KEY_W, KEY_X, KEY_Y, KEY_Z,

    KEY_RETURN,
    KEY_ESCAPE,
    KEY_TAB,
    KEY_BACKSPACE,
    KEY_SPACE,

    KEY_ADD,
    KEY_SUBTRACT,
    KEY_COMMA,
    KEY_LEFTBRACKET,
    KEY_RIGHTBRACKET,
    KEY_DECIMAL,
    KEY_SEMICOLON,
    KEY_PERIOD,
    KEY_SLASH,
    KEY_SINGLEQUOTE,
    KEY_BACKSLASH,
    KEY_TILDE,

    KEY_LEFT,
    KEY_RIGHT,
    KEY_UP,
    KEY_DOWN,

    KEY_PRINTSCREEN,

    KEY_INSERT,
    KEY_HOME,
    KEY_END,
    KEY_PAGEUP,
    KEY_PAGEDOWN,
    KEY_DELETE,
    KEY_SELECT,

    KEY_CONTROL,
    KEY_ALT,
    KEY_SHIFT,

    KEY_NUMPAD_0, KEY_NUMPAD_1, KEY_NUMPAD_2, KEY_NUMPAD_3, KEY_NUMPAD_4, KEY_NUMPAD_5, KEY_NUMPAD_6, KEY_NUMPAD_7, KEY_NUMPAD_8, KEY_NUMPAD_9,

    KEY_GAMEPAD_A,
    KEY_GAMEPAD_B,
    KEY_GAMEPAD_X,
    KEY_GAMEPAD_Y,
    KEY_GAMEPAD_LEFT,
    KEY_GAMEPAD_RIGHT,
    KEY_GAMEPAD_UP,
    KEY_GAMEPAD_DOWN,
    KEY_GAMEPAD_L1,
    KEY_GAMEPAD_R1,
    KEY_GAMEPAD_L3,
    KEY_GAMEPAD_R3 ,
    KEY_GAMEPAD_START,
    KEY_GAMEPAD_BACK,
    KEY_GAMEPAD_GUIDE,
    KEY_GAMEPAD_TOUCHPAD,

    KEY_GAMEPAD_LEFTSTICK_LEFT,
    KEY_GAMEPAD_LEFTSTICK_RIGHT,
    KEY_GAMEPAD_LEFTSTICK_UP,
    KEY_GAMEPAD_LEFTSTICK_DOWN,
    KEY_GAMEPAD_RIGHTSTICK_LEFT,
    KEY_GAMEPAD_RIGHTSTICK_RIGHT,
    KEY_GAMEPAD_RIGHTSTICK_UP,
    KEY_GAMEPAD_RIGHTSTICK_DOWN,
    KEY_GAMEPAD_L2,
    KEY_GAMEPAD_R2,

    KEY_NONE,

    KEY_FIRST = KEY_CHAR,
    KEY_LAST = KEY_GAMEPAD_R2,
};

enum class PlatformModifier : uint32_t {
    MOD_SHIFT   = 0x00000001,
    MOD_CTRL    = 0x00000002,
    MOD_ALT     = 0x00000004,
    MOD_META    = 0x00000008,
    MOD_NUM     = 0x00000010,
};
using enum PlatformModifier;
MM_DECLARE_FLAGS(PlatformModifiers, PlatformModifier)
MM_DECLARE_OPERATORS_FOR_FLAGS(PlatformModifiers)

enum class PlatformMouseButton {
    BUTTON_NONE     = 0,
    BUTTON_LEFT     = 0x1,
    BUTTON_MIDDLE   = 0x2,
    BUTTON_RIGHT    = 0x4
};
using enum PlatformMouseButton;
MM_DECLARE_FLAGS(PlatformMouseButtons, PlatformMouseButton)
MM_DECLARE_OPERATORS_FOR_FLAGS(PlatformMouseButtons)

/**
 * - `WINDOW_MODE_WINDOWED` is a normal window with frame.
 * - `WINDOW_MODE_BORDERLESS` is a window without frame.
 * - `WINDOW_MODE_FULLSCREEN` is a traditional exclusive fullscreen mode. It changes display resolution and makes
 *   the window the sole user of the display.
 * - `WINDOW_MODE_FULLSCREEN_BORDERLESS` is a modern fake fullscreen AKA frameless window resized to desktop resolution
 *   and moved to cover the whole desktop.
 */
enum class PlatformWindowMode {
    WINDOW_MODE_WINDOWED = 0,
    WINDOW_MODE_BORDERLESS = 1,
    WINDOW_MODE_FULLSCREEN = 2,
    WINDOW_MODE_FULLSCREEN_BORDERLESS = 3,
};
using enum PlatformWindowMode;

enum class PlatformWindowOrientation {
    LANDSCAPE_LEFT = 0x01,
    LANDSCAPE_RIGHT = 0x02,
    PORTRAIT_UP = 0x04,
    PORTRAIT_DOWN = 0x08,
};
using enum PlatformWindowOrientation;
MM_DECLARE_FLAGS(PlatformWindowOrientations, PlatformWindowOrientation)
MM_DECLARE_OPERATORS_FOR_FLAGS(PlatformWindowOrientations)
