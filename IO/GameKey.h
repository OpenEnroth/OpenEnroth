#pragma once

#include <string>

namespace Io {

    // dont forget to add new values to sdl mappings
    enum class GameKey : int {
        // usual text input
        Char,

        // scancodes (langauge and case neutral)
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

        Digit1, Digit2, Digit3, Digit4, Digit5, Digit6, Digit7, Digit8, Digit9, Digit0,
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

        Numpad0,

        None
    };
}

/// Some GameKeys are also used in textbox input so we should send them as char input as well
inline bool IsKeyUsedInTextInput(Io::GameKey key) {
    return key == Io::GameKey::Return
        || key == Io::GameKey::Escape
        || key == Io::GameKey::Backspace;
}

inline bool IsKeyAlphaNumberic(Io::GameKey key) {
    return key >= Io::GameKey::Digit1 && key <= Io::GameKey::Digit0
        || key >= Io::GameKey::A && key <= Io::GameKey::Z;
}

std::string GetDisplayName(Io::GameKey key);
bool TryParseDisplayName(const std::string &displayName, Io::GameKey *outKey);