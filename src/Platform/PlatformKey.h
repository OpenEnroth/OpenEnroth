#pragma once

#include <string>
#include <string_view>

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

typedef float PlatformKeyValue;
