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

    Count,

    None = Count
};

std::string GetDisplayName(PlatformKey key);
bool TryParseDisplayName(std::string_view displayName, PlatformKey *outKey);
