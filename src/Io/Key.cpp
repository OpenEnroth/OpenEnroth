#include "Key.h"

#include "Library/Serialization/EnumSerialization.h"
#include "Library/Serialization/EnumSerializer.h"
#include "Library/Serialization/Serialization.h"
#include "Platform/PlatformEnums.h"

// TODO(captainurist): recheck that ALL keys are actually serializable
MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(PlatformKey, CASE_INSENSITIVE, {
    {PlatformKey::KEY_F1,           "F1" },
    {PlatformKey::KEY_F2,           "F2" },
    {PlatformKey::KEY_F3,           "F3" },
    {PlatformKey::KEY_F4,           "F4" },
    {PlatformKey::KEY_F5,           "F5" },
    {PlatformKey::KEY_F6,           "F6" },
    {PlatformKey::KEY_F7,           "F7" },
    {PlatformKey::KEY_F8,           "F8" },
    {PlatformKey::KEY_F9,           "F9" },
    {PlatformKey::KEY_F10,          "F10" },
    {PlatformKey::KEY_F11,          "F11" },
    {PlatformKey::KEY_F12,          "F12" },

    {PlatformKey::KEY_A,            "A" },
    {PlatformKey::KEY_B,            "B" },
    {PlatformKey::KEY_C,            "C" },
    {PlatformKey::KEY_D,            "D" },
    {PlatformKey::KEY_E,            "E" },
    {PlatformKey::KEY_F,            "F" },
    {PlatformKey::KEY_G,            "G" },
    {PlatformKey::KEY_H,            "H" },
    {PlatformKey::KEY_I,            "I" },
    {PlatformKey::KEY_J,            "J" },
    {PlatformKey::KEY_K,            "K" },
    {PlatformKey::KEY_L,            "L" },
    {PlatformKey::KEY_M,            "M" },
    {PlatformKey::KEY_N,            "N" },
    {PlatformKey::KEY_O,            "O" },
    {PlatformKey::KEY_P,            "P" },
    {PlatformKey::KEY_Q,            "Q" },
    {PlatformKey::KEY_R,            "R" },
    {PlatformKey::KEY_S,            "S" },
    {PlatformKey::KEY_T,            "T" },
    {PlatformKey::KEY_U,            "U" },
    {PlatformKey::KEY_V,            "V" },
    {PlatformKey::KEY_W,            "W" },
    {PlatformKey::KEY_X,            "X" },
    {PlatformKey::KEY_Y,            "Y" },
    {PlatformKey::KEY_Z,            "Z" },

    {PlatformKey::KEY_DIGIT_1,       "1" },
    {PlatformKey::KEY_DIGIT_2,       "2" },
    {PlatformKey::KEY_DIGIT_3,       "3" },
    {PlatformKey::KEY_DIGIT_4,       "4" },
    {PlatformKey::KEY_DIGIT_5,       "5" },
    {PlatformKey::KEY_DIGIT_6,       "6" },
    {PlatformKey::KEY_DIGIT_7,       "7" },
    {PlatformKey::KEY_DIGIT_8,       "8" },
    {PlatformKey::KEY_DIGIT_9,       "9" },
    {PlatformKey::KEY_DIGIT_0,       "0" },

    {PlatformKey::KEY_UP,           "UP" },
    {PlatformKey::KEY_DOWN,         "DOWN" },
    {PlatformKey::KEY_LEFT,         "LEFT" },
    {PlatformKey::KEY_RIGHT,        "RIGHT" },
    {PlatformKey::KEY_RETURN,       "RETURN" },
    {PlatformKey::KEY_ESCAPE,       "ESCAPE" },
    {PlatformKey::KEY_SPACE,        "SPACE" },
    {PlatformKey::KEY_PAGEDOWN,     "PAGE DOWN" },
    {PlatformKey::KEY_PAGEUP,       "PAGE UP" },
    {PlatformKey::KEY_TAB,          "TAB" },
    {PlatformKey::KEY_SUBTRACT,     "SUBTRACT" },
    {PlatformKey::KEY_ADD,          "ADD" },
    {PlatformKey::KEY_END,          "END" },
    {PlatformKey::KEY_DELETE,       "DELETE" },
    {PlatformKey::KEY_HOME,         "HOME" },
    {PlatformKey::KEY_INSERT,       "INSERT" },
    {PlatformKey::KEY_COMMA,        "COMMA" },
    {PlatformKey::KEY_DECIMAL,      "DECIMAL" },
    {PlatformKey::KEY_SEMICOLON,    "SEMICOLON" },
    {PlatformKey::KEY_PERIOD,       "PERIOD" },
    {PlatformKey::KEY_SLASH,        "SLASH" },
    {PlatformKey::KEY_SINGLEQUOTE,  "SQUOTE" },
    {PlatformKey::KEY_BACKSLASH,    "BACKSLASH" },
    {PlatformKey::KEY_BACKSPACE,    "BACKSPACE" },
    {PlatformKey::KEY_LEFTBRACKET,  "L BRACKET" },
    {PlatformKey::KEY_RIGHTBRACKET, "R BRACKET" },
    {PlatformKey::KEY_TILDE,        "~" },

    {PlatformKey::KEY_CONTROL,      "CONTROL"},
    {PlatformKey::KEY_ALT,          "ALT"},
    {PlatformKey::KEY_SHIFT,        "SHIFT"},

    {PlatformKey::KEY_NUMPAD_0,      "NUMPAD 0" },
    {PlatformKey::KEY_NUMPAD_1,      "NUMPAD 1" },
    {PlatformKey::KEY_NUMPAD_2,      "NUMPAD 2" },
    {PlatformKey::KEY_NUMPAD_3,      "NUMPAD 3" },
    {PlatformKey::KEY_NUMPAD_4,      "NUMPAD 4" },
    {PlatformKey::KEY_NUMPAD_5,      "NUMPAD 5" },
    {PlatformKey::KEY_NUMPAD_6,      "NUMPAD 6" },
    {PlatformKey::KEY_NUMPAD_7,      "NUMPAD 7" },
    {PlatformKey::KEY_NUMPAD_8,      "NUMPAD 8" },
    {PlatformKey::KEY_NUMPAD_9,      "NUMPAD 9" },

    {PlatformKey::KEY_GAMEPAD_A,        "A (CROSS)"},
    {PlatformKey::KEY_GAMEPAD_B,        "B (CIRCLE)"},
    {PlatformKey::KEY_GAMEPAD_X,        "X (SQUARE)"},
    {PlatformKey::KEY_GAMEPAD_Y,        "Y (TRIANGLE)"},
    {PlatformKey::KEY_GAMEPAD_LEFT,     "DPAD LEFT"},
    {PlatformKey::KEY_GAMEPAD_RIGHT,    "DPAD RIGHT"},
    {PlatformKey::KEY_GAMEPAD_UP,       "DPAD UP"},
    {PlatformKey::KEY_GAMEPAD_DOWN,     "DPAD DOWN"},
    {PlatformKey::KEY_GAMEPAD_L1,       "L1"},
    {PlatformKey::KEY_GAMEPAD_R1,       "R1"},
    {PlatformKey::KEY_GAMEPAD_L3,       "L3"},
    {PlatformKey::KEY_GAMEPAD_R3,       "R3"},
    {PlatformKey::KEY_GAMEPAD_START,    "START"},
    {PlatformKey::KEY_GAMEPAD_BACK,     "BACK"},
    {PlatformKey::KEY_GAMEPAD_GUIDE,    "GUIDE"},
    {PlatformKey::KEY_GAMEPAD_TOUCHPAD, "TOUCHPAD"},

    {PlatformKey::KEY_GAMEPAD_LEFTSTICK_LEFT,   "LSTICK LEFT"},
    {PlatformKey::KEY_GAMEPAD_LEFTSTICK_RIGHT,  "LSTICK RIGHT"},
    {PlatformKey::KEY_GAMEPAD_LEFTSTICK_UP,     "LSTICK UP"},
    {PlatformKey::KEY_GAMEPAD_LEFTSTICK_DOWN,   "LSTICK DOWN"},
    {PlatformKey::KEY_GAMEPAD_RIGHTSTICK_LEFT,  "RSTICK LEFT"},
    {PlatformKey::KEY_GAMEPAD_RIGHTSTICK_RIGHT, "RSTICK RIGHT"},
    {PlatformKey::KEY_GAMEPAD_RIGHTSTICK_UP,    "RSTICK UP"},
    {PlatformKey::KEY_GAMEPAD_RIGHTSTICK_DOWN,  "RSTICK DOWN"},
    {PlatformKey::KEY_GAMEPAD_L2,               "L2"},
    {PlatformKey::KEY_GAMEPAD_R2,               "R2"},
    {PlatformKey::KEY_NONE,                     ""},
    {PlatformKey::KEY_NONE,                     "-NOT-SET-"} // Allowed for deserialization only, relied on in TryParseDisplayName
})

std::string GetDisplayName(PlatformKey key) {
    if (key == PlatformKey::KEY_NONE)
        return "-NOT-SET-";
    return toString(key); // Should always work.
}

bool TryParseDisplayName(std::string_view displayName, PlatformKey *outKey) {
    return tryDeserialize(displayName, outKey);
}
