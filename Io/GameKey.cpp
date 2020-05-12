#include <map>
#include <utility>

#include "Io/GameKey.h"

#ifndef _WINDOWS
#define _stricmp strcasecmp
#endif

using Io::GameKey;


static std::map<GameKey, const char*> displayNames = {
    { GameKey::F1,              "F1" },
    { GameKey::F2,              "F2" },
    { GameKey::F3,              "F3" },
    { GameKey::F4,              "F4" },
    { GameKey::F5,              "F5" },
    { GameKey::F6,              "F6" },
    { GameKey::F7,              "F7" },
    { GameKey::F8,              "F8" },
    { GameKey::F9,              "F9" },
    { GameKey::F10,             "F10" },
    { GameKey::F11,             "F11" },
    { GameKey::F12,             "F12" },

    { GameKey::A,               "A" },
    { GameKey::B,               "B" },
    { GameKey::C,               "C" },
    { GameKey::D,               "D" },
    { GameKey::E,               "E" },
    { GameKey::F,               "F" },
    { GameKey::G,               "G" },
    { GameKey::H,               "H" },
    { GameKey::I,               "I" },
    { GameKey::J,               "J" },
    { GameKey::K,               "K" },
    { GameKey::L,               "L" },
    { GameKey::M,               "M" },
    { GameKey::N,               "N" },
    { GameKey::O,               "O" },
    { GameKey::P,               "P" },
    { GameKey::Q,               "Q" },
    { GameKey::R,               "R" },
    { GameKey::S,               "S" },
    { GameKey::T,               "T" },
    { GameKey::U,               "U" },
    { GameKey::V,               "V" },
    { GameKey::W,               "W" },
    { GameKey::X,               "X" },
    { GameKey::Y,               "Y" },
    { GameKey::Z,               "Z" },

    { GameKey::Digit1,          "1" },
    { GameKey::Digit2,          "2" },
    { GameKey::Digit3,          "3" },
    { GameKey::Digit4,          "4" },
    { GameKey::Digit5,          "5" },
    { GameKey::Digit6,          "6" },
    { GameKey::Digit7,          "7" },
    { GameKey::Digit8,          "8" },
    { GameKey::Digit9,          "9" },
    { GameKey::Digit0,          "0" },

    { GameKey::Up,              "UP" },
    { GameKey::Down,            "DOWN" },
    { GameKey::Left,            "LEFT" },
    { GameKey::Right,           "RIGHT" },
    { GameKey::Return,          "RETURN" },
    { GameKey::Space,           "SPACE" },
    { GameKey::PageDown,        "PAGE DOWN" },
    { GameKey::PageUp,          "PAGE UP" },
    { GameKey::Tab,             "TAB" },
    { GameKey::Subtract,        "SUBTRACT" },
    { GameKey::Add,             "ADD" },
    { GameKey::End,             "END" },
    { GameKey::Delete,          "DELETE" },
    { GameKey::Home,            "HOME" },
    { GameKey::Insert,          "INSERT" },
    { GameKey::Comma,           "COMMA" },
    { GameKey::Decimal,         "DECIMAL" },
    { GameKey::Semicolon,       "SEMICOLON" },
    { GameKey::Period,          "PERIOD" },
    { GameKey::Slash,           "SLASH" },
    { GameKey::SingleQuote,     "SQUOTE" },
    { GameKey::BackSlash,       "BACKSLASH" },
    { GameKey::Backspace,       "BACKSPACE" },
    { GameKey::Control,         "CONTROL" },
    { GameKey::LeftBracket,     "L BRACKET" },
    { GameKey::RightBracket,    "R BRACKET" },
    { GameKey::Tilde,           "~" },
};

std::string GetDisplayName(GameKey key) {
    auto displayName = displayNames.find(key);
    if (displayName != displayNames.end()) {
        return displayName->second;
    }

    return "-NOT-SET-";
}

bool TryParseDisplayName(const std::string &displayName, GameKey *outKey) {
    const char *displayNameStr = displayName.c_str();

    auto displayNameMapping = std::find_if(
        displayNames.begin(),
        displayNames.end(),
        [displayNameStr](const std::pair<GameKey, const char *> &i) -> bool {
            // TODO refactor this using STL to void using _stricmp and its pesky #ifndef WINDOWS
            return _stricmp(displayNameStr, i.second) == 0;
        }
    );

    if (displayNameMapping != displayNames.end()) {
        if (outKey != nullptr) {
            *outKey = displayNameMapping->first;
        }
        return true;
    }

    return false;
}
