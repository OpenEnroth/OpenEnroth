#include <cassert>
#include <unordered_map>

#include "Utility/String.h"
#include "Utility/MapAccess.h"

#include "PlatformKey.h"

static std::unordered_map<PlatformKey, std::string> displayNameByKey = {
    {PlatformKey::F1,           "F1" },
    {PlatformKey::F2,           "F2" },
    {PlatformKey::F3,           "F3" },
    {PlatformKey::F4,           "F4" },
    {PlatformKey::F5,           "F5" },
    {PlatformKey::F6,           "F6" },
    {PlatformKey::F7,           "F7" },
    {PlatformKey::F8,           "F8" },
    {PlatformKey::F9,           "F9" },
    {PlatformKey::F10,          "F10" },
    {PlatformKey::F11,          "F11" },
    {PlatformKey::F12,          "F12" },

    {PlatformKey::A,            "A" },
    {PlatformKey::B,            "B" },
    {PlatformKey::C,            "C" },
    {PlatformKey::D,            "D" },
    {PlatformKey::E,            "E" },
    {PlatformKey::F,            "F" },
    {PlatformKey::G,            "G" },
    {PlatformKey::H,            "H" },
    {PlatformKey::I,            "I" },
    {PlatformKey::J,            "J" },
    {PlatformKey::K,            "K" },
    {PlatformKey::L,            "L" },
    {PlatformKey::M,            "M" },
    {PlatformKey::N,            "N" },
    {PlatformKey::O,            "O" },
    {PlatformKey::P,            "P" },
    {PlatformKey::Q,            "Q" },
    {PlatformKey::R,            "R" },
    {PlatformKey::S,            "S" },
    {PlatformKey::T,            "T" },
    {PlatformKey::U,            "U" },
    {PlatformKey::V,            "V" },
    {PlatformKey::W,            "W" },
    {PlatformKey::X,            "X" },
    {PlatformKey::Y,            "Y" },
    {PlatformKey::Z,            "Z" },

    {PlatformKey::Digit1,       "1" },
    {PlatformKey::Digit2,       "2" },
    {PlatformKey::Digit3,       "3" },
    {PlatformKey::Digit4,       "4" },
    {PlatformKey::Digit5,       "5" },
    {PlatformKey::Digit6,       "6" },
    {PlatformKey::Digit7,       "7" },
    {PlatformKey::Digit8,       "8" },
    {PlatformKey::Digit9,       "9" },
    {PlatformKey::Digit0,       "0" },

    {PlatformKey::Up,           "UP" },
    {PlatformKey::Down,         "DOWN" },
    {PlatformKey::Left,         "LEFT" },
    {PlatformKey::Right,        "RIGHT" },
    {PlatformKey::Return,       "RETURN" },
    {PlatformKey::Space,        "SPACE" },
    {PlatformKey::PageDown,     "PAGE DOWN" },
    {PlatformKey::PageUp,       "PAGE UP" },
    {PlatformKey::Tab,          "TAB" },
    {PlatformKey::Subtract,     "SUBTRACT" },
    {PlatformKey::Add,          "ADD" },
    {PlatformKey::End,          "END" },
    {PlatformKey::Delete,       "DELETE" },
    {PlatformKey::Home,         "HOME" },
    {PlatformKey::Insert,       "INSERT" },
    {PlatformKey::Comma,        "COMMA" },
    {PlatformKey::Decimal,      "DECIMAL" },
    {PlatformKey::Semicolon,    "SEMICOLON" },
    {PlatformKey::Period,       "PERIOD" },
    {PlatformKey::Slash,        "SLASH" },
    {PlatformKey::SingleQuote,  "SQUOTE" },
    {PlatformKey::BackSlash,    "BACKSLASH" },
    {PlatformKey::Backspace,    "BACKSPACE" },
    {PlatformKey::Control,      "CONTROL" },
    {PlatformKey::LeftBracket,  "L BRACKET" },
    {PlatformKey::RightBracket, "R BRACKET" },
    {PlatformKey::Tilde,        "~" },

    {PlatformKey::Numpad0,      "NUMPAD 0" },
    {PlatformKey::Numpad1,      "NUMPAD 1" },
    {PlatformKey::Numpad2,      "NUMPAD 2" },
    {PlatformKey::Numpad3,      "NUMPAD 3" },
    {PlatformKey::Numpad4,      "NUMPAD 4" },
    {PlatformKey::Numpad5,      "NUMPAD 5" },
    {PlatformKey::Numpad6,      "NUMPAD 6" },
    {PlatformKey::Numpad7,      "NUMPAD 7" },
    {PlatformKey::Numpad8,      "NUMPAD 8" },
    {PlatformKey::Numpad9,      "NUMPAD 9" },
};

static std::unordered_map<std::string, PlatformKey> keyByDisplayName = Inverted(displayNameByKey);

std::string GetDisplayName(PlatformKey key) {
    return ValueOr(displayNameByKey, key, "-NOT-SET-"); // TODO(captainurist): "NOT-SET" doesn't belong here.
}

bool TryParseDisplayName(std::string_view displayName, PlatformKey *outKey) {
    assert(outKey);

    *outKey = ValueOr(keyByDisplayName, ToUpper(displayName), PlatformKey::None);
    return *outKey != PlatformKey::None;
}
