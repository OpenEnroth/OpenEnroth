#pragma once

#include <cstdint>

#include "Utility/Flags.h"

enum class PLATFORM_STORAGE {
    ANDROID_STORAGE_INTERNAL,
    ANDROID_STORAGE_EXTERNAL
};
using enum PLATFORM_STORAGE;

/**
 * Platform log level as used by `PlatformLogger`.
 */
enum class PlatformLogLevel {
    LogVerbose,
    LogDebug,
    LogInfo,
    LogWarning,
    LogError,
    LogCritical
};
using enum PlatformLogLevel;

/**
 * Platform log category as used by `PlatformLogger`.
 *
 * Note that platform doesn't have an API to define custom log categories, this should be done in user code if needed.
 */
enum class PlatformLogCategory {
    PlatformLog,
    ApplicationLog
};
using enum PlatformLogCategory;

enum PlatformLoggerOption {
    WinEnsureConsoleOption = 0x1
};
using enum PlatformLoggerOption;
MM_DECLARE_FLAGS(PlatformLoggerOptions, PlatformLoggerOption)
MM_DECLARE_OPERATORS_FOR_FLAGS(PlatformLoggerOptions)

enum class PlatformKey : int {
    // usual text input
    Char,

    // scancodes (language and case neutral)
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,

    Digit0, Digit1, Digit2, Digit3, Digit4, Digit5, Digit6, Digit7, Digit8, Digit9,
    A, B, C, D, E, F, G,
    H, I, J, K, L, M, N, O, P,
    Q, R, S,
    T, U, V,
    W, X, Y, Z,

    Return,
    Escape,
    Tab,
    Backspace,
    Space,

    Add,
    Subtract,
    Comma,
    LeftBracket,
    RightBracket,
    Decimal,
    Semicolon,
    Period,
    Slash,
    SingleQuote,
    BackSlash,
    Tilde,

    Left,
    Right,
    Up,
    Down,

    PrintScreen,

    Insert,
    Home,
    End,
    PageUp,
    PageDown,
    Delete,
    Select,

    Control,
    Alt,
    Shift,

    Numpad0, Numpad1, Numpad2, Numpad3, Numpad4, Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,

    Gamepad_A,
    Gamepad_B,
    Gamepad_X,
    Gamepad_Y,
    Gamepad_Left,
    Gamepad_Right,
    Gamepad_Up,
    Gamepad_Down,
    Gamepad_L1,
    Gamepad_R1,
    Gamepad_L3,
    Gamepad_R3 ,
    Gamepad_Start,
    Gamepad_Back,
    Gamepad_Guide,
    Gamepad_Touchpad,

    Gamepad_LeftStick_Left,
    Gamepad_LeftStick_Right,
    Gamepad_LeftStick_Up,
    Gamepad_LeftStick_Down,
    Gamepad_RightStick_Left,
    Gamepad_RightStick_Right,
    Gamepad_RightStick_Up,
    Gamepad_RightStick_Down,
    Gamepad_L2,
    Gamepad_R2,

    Count,

    None = Count
};

enum class PlatformKeyType : int {
    KEY_TYPE_KEYBOARD_BUTTON,
    KEY_TYPE_GAMEPAD_BUTTON,
    KEY_TYPE_GAMEPAD_AXIS,
    KEY_TYPE_GAMEPAD_TRIGGER
};
using enum PlatformKeyType;

typedef float PlatformKeyValue;

enum class PlatformModifier : uint32_t {
    Shift  = 0x00000001,
    Ctrl   = 0x00000002,
    Alt    = 0x00000004,
    Meta   = 0x00000008,
    NumPad = 0x00000010,
};

MM_DECLARE_FLAGS(PlatformModifiers, PlatformModifier)
MM_DECLARE_OPERATORS_FOR_FLAGS(PlatformModifiers)

enum class PlatformMouseButton {
    None    = 0,
    Left    = 0x1,
    Middle  = 0x2,
    Right   = 0x4
};

MM_DECLARE_FLAGS(PlatformMouseButtons, PlatformMouseButton)
MM_DECLARE_OPERATORS_FOR_FLAGS(PlatformMouseButtons)

/**
 * - `WINDOW_MODE_WINDOWED` is a normal window with frame.
 * - `WINDOW_MODE_BORDERLESS` is a window without frame.
 * - `WINDOW_MODE_FULLSCREEN` is a traditional exclusive fullscreen mode. It changes display resolution and makes
 *   the window the sole user of the display.
 * - `WINDOW_MODE_FAKE_FULLSCREEN` is a modern fake fullscreen AKA frameless window resized to desktop resolution
 *   and moved to cover the whole desktop.
 */
enum class PlatformWindowMode {
    WINDOW_MODE_WINDOWED = 0,
    WINDOW_MODE_BORDERLESS = 1,
    WINDOW_MODE_FULLSCREEN = 2,
    WINDOW_MODE_FAKE_FULLSCREEN = 3,
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
