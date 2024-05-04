#include "InputBindings.h"

#include <string>
#include <optional>

#include "Library/Logger/Logger.h"

#include "InputScriptEventHandler.h"

InputBindings::InputBindings(InputScriptEventHandler &inputScriptEventHandler)
    : _inputScriptEventHandler(inputScriptEventHandler) {
    /*_inputScriptEventHandler.setScriptFunctionProvider([this](std::string_view functionName) {
        return _solState[functionName];
    });*/
}

sol::table InputBindings::createBindingTable(sol::state_view &solState) const {
    sol::table table = solState.create_table();
    _fillTableWithEnums(table);
    return table;
}

void InputBindings::_fillTableWithEnums(sol::table &table) const {
    table.new_enum<false>("PlatformKey",
        /*
        "KEY_CHAR", PlatformKey::KEY_CHAR,
        "KEY_F1", PlatformKey::KEY_F1,
        "KEY_F2", PlatformKey::KEY_F2,
        "KEY_F3", PlatformKey::KEY_F3,
        "KEY_F4", PlatformKey::KEY_F4,
        "KEY_F5", PlatformKey::KEY_F5,
        "KEY_F6", PlatformKey::KEY_F6,
        "KEY_F7", PlatformKey::KEY_F7,
        "KEY_F8", PlatformKey::KEY_F8,
        "KEY_F9", PlatformKey::KEY_F9,
        "KEY_F10", PlatformKey::KEY_F10,
        "KEY_F11", PlatformKey::KEY_F11,
        "KEY_F12", PlatformKey::KEY_F12,
        "KEY_F13", PlatformKey::KEY_F13,
        "KEY_F14", PlatformKey::KEY_F14,
        "KEY_F15", PlatformKey::KEY_F15,
        "KEY_F16", PlatformKey::KEY_F16,
        "KEY_F17", PlatformKey::KEY_F17,
        "KEY_F18", PlatformKey::KEY_F18,
        "KEY_F19", PlatformKey::KEY_F19,
        "KEY_F20", PlatformKey::KEY_F20,
        "KEY_F21", PlatformKey::KEY_F21,
        "KEY_F22", PlatformKey::KEY_F22,
        "KEY_F23", PlatformKey::KEY_F23,
        "KEY_F24", PlatformKey::KEY_F24,

        "KEY_DIGIT_0", PlatformKey::KEY_DIGIT_0, "KEY_DIGIT_1", PlatformKey::KEY_DIGIT_1, "KEY_DIGIT_2", PlatformKey::KEY_DIGIT_2,
        "KEY_DIGIT_3", PlatformKey::KEY_DIGIT_3, "KEY_DIGIT_4", PlatformKey::KEY_DIGIT_4, "KEY_DIGIT_5", PlatformKey::KEY_DIGIT_5,
        "KEY_DIGIT_6", PlatformKey::KEY_DIGIT_6, "KEY_DIGIT_7", PlatformKey::KEY_DIGIT_7, "KEY_DIGIT_8", PlatformKey::KEY_DIGIT_8,
        "KEY_DIGIT_9", PlatformKey::KEY_DIGIT_9,

        "KEY_A", PlatformKey::KEY_A, "KEY_B", PlatformKey::KEY_B, "KEY_C", PlatformKey::KEY_C, "KEY_D", PlatformKey::KEY_D,
        "KEY_E", PlatformKey::KEY_E, "KEY_F", PlatformKey::KEY_F, "KEY_G", PlatformKey::KEY_G, "KEY_H", PlatformKey::KEY_H,
        "KEY_I", PlatformKey::KEY_I, "KEY_J", PlatformKey::KEY_J, "KEY_K", PlatformKey::KEY_K, "KEY_L", PlatformKey::KEY_L,
        "KEY_M", PlatformKey::KEY_M, "KEY_N", PlatformKey::KEY_N, "KEY_O", PlatformKey::KEY_O, "KEY_P", PlatformKey::KEY_P,
        "KEY_Q", PlatformKey::KEY_Q, "KEY_R", PlatformKey::KEY_R, "KEY_S", PlatformKey::KEY_S, "KEY_T", PlatformKey::KEY_T,
        "KEY_U", PlatformKey::KEY_U, "KEY_V", PlatformKey::KEY_V, "KEY_W", PlatformKey::KEY_W, "KEY_X", PlatformKey::KEY_X,
        "KEY_Y", PlatformKey::KEY_Y, "KEY_Z", PlatformKey::KEY_Z,

        "KEY_RETURN", PlatformKey::KEY_RETURN,
        "KEY_ESCAPE", PlatformKey::KEY_ESCAPE,
        "KEY_TAB", PlatformKey::KEY_TAB,
        "KEY_BACKSPACE", PlatformKey::KEY_BACKSPACE,
        "KEY_SPACE", PlatformKey::KEY_SPACE,

        "KEY_ADD", PlatformKey::KEY_ADD,
        "KEY_SUBTRACT", PlatformKey::KEY_SUBTRACT,
        "KEY_COMMA", PlatformKey::KEY_COMMA,
        "KEY_LEFTBRACKET", PlatformKey::KEY_LEFTBRACKET,
        "KEY_RIGHTBRACKET", PlatformKey::KEY_RIGHTBRACKET,
        "KEY_DECIMAL", PlatformKey::KEY_DECIMAL,
        "KEY_SEMICOLON", PlatformKey::KEY_SEMICOLON,
        "KEY_PERIOD", PlatformKey::KEY_PERIOD,
        "KEY_SLASH", PlatformKey::KEY_SLASH,
        "KEY_SINGLEQUOTE", PlatformKey::KEY_SINGLEQUOTE,
        "KEY_BACKSLASH", PlatformKey::KEY_BACKSLASH,
        "KEY_TILDE", PlatformKey::KEY_TILDE,
        */
        "KEY_LEFT", PlatformKey::KEY_LEFT,
        "KEY_RIGHT", PlatformKey::KEY_RIGHT,
        "KEY_UP", PlatformKey::KEY_UP,
        "KEY_DOWN", PlatformKey::KEY_DOWN
        /*
        "KEY_PRINTSCREEN", PlatformKey::KEY_PRINTSCREEN,

        "KEY_INSERT", PlatformKey::KEY_INSERT,
        "KEY_HOME", PlatformKey::KEY_HOME,
        "KEY_END", PlatformKey::KEY_END,
        "KEY_PAGEUP", PlatformKey::KEY_PAGEUP,
        "KEY_PAGEDOWN", PlatformKey::KEY_PAGEDOWN,
        "KEY_DELETE", PlatformKey::KEY_DELETE,
        "KEY_SELECT", PlatformKey::KEY_SELECT,

        "KEY_CONTROL", PlatformKey::KEY_CONTROL,
        "KEY_ALT", PlatformKey::KEY_ALT,
        "KEY_SHIFT", PlatformKey::KEY_SHIFT,

        "KEY_NUMPAD_0", PlatformKey::KEY_NUMPAD_0, "KEY_NUMPAD_1", PlatformKey::KEY_NUMPAD_1, "KEY_NUMPAD_2", PlatformKey::KEY_NUMPAD_2,
        "KEY_NUMPAD_3", PlatformKey::KEY_NUMPAD_3, "KEY_NUMPAD_4", PlatformKey::KEY_NUMPAD_4, "KEY_NUMPAD_5", PlatformKey::KEY_NUMPAD_5,
        "KEY_NUMPAD_6", PlatformKey::KEY_NUMPAD_6, "KEY_NUMPAD_7", PlatformKey::KEY_NUMPAD_7, "KEY_NUMPAD_8", PlatformKey::KEY_NUMPAD_8,
        "KEY_NUMPAD_9", PlatformKey::KEY_NUMPAD_9,

        "KEY_GAMEPAD_A", PlatformKey::KEY_GAMEPAD_A,
        "KEY_GAMEPAD_B", PlatformKey::KEY_GAMEPAD_B,
        "KEY_GAMEPAD_X", PlatformKey::KEY_GAMEPAD_X,
        "KEY_GAMEPAD_Y", PlatformKey::KEY_GAMEPAD_Y,
        "KEY_GAMEPAD_LEFT", PlatformKey::KEY_GAMEPAD_LEFT,
        "KEY_GAMEPAD_RIGHT", PlatformKey::KEY_GAMEPAD_RIGHT,
        "KEY_GAMEPAD_UP", PlatformKey::KEY_GAMEPAD_UP,
        "KEY_GAMEPAD_DOWN", PlatformKey::KEY_GAMEPAD_DOWN,
        "KEY_GAMEPAD_L1", PlatformKey::KEY_GAMEPAD_L1,
        "KEY_GAMEPAD_R1", PlatformKey::KEY_GAMEPAD_R1,
        "KEY_GAMEPAD_L3", PlatformKey::KEY_GAMEPAD_L3,
        "KEY_GAMEPAD_R3", PlatformKey::KEY_GAMEPAD_R3,
        "KEY_GAMEPAD_START", PlatformKey::KEY_GAMEPAD_START,
        "KEY_GAMEPAD_BACK", PlatformKey::KEY_GAMEPAD_BACK,
        "KEY_GAMEPAD_GUIDE", PlatformKey::KEY_GAMEPAD_GUIDE,
        "KEY_GAMEPAD_TOUCHPAD", PlatformKey::KEY_GAMEPAD_TOUCHPAD,

        "KEY_GAMEPAD_LEFTSTICK_LEFT", PlatformKey::KEY_GAMEPAD_LEFTSTICK_LEFT,
        "KEY_GAMEPAD_LEFTSTICK_RIGHT", PlatformKey::KEY_GAMEPAD_LEFTSTICK_RIGHT,
        "KEY_GAMEPAD_LEFTSTICK_UP", PlatformKey::KEY_GAMEPAD_LEFTSTICK_UP,
        "KEY_GAMEPAD_LEFTSTICK_DOWN", PlatformKey::KEY_GAMEPAD_LEFTSTICK_DOWN,
        "KEY_GAMEPAD_RIGHTSTICK_LEFT", PlatformKey::KEY_GAMEPAD_RIGHTSTICK_LEFT,
        "KEY_GAMEPAD_RIGHTSTICK_RIGHT", PlatformKey::KEY_GAMEPAD_RIGHTSTICK_RIGHT,
        "KEY_GAMEPAD_RIGHTSTICK_UP", PlatformKey::KEY_GAMEPAD_RIGHTSTICK_UP,
        "KEY_GAMEPAD_RIGHTSTICK_DOWN", PlatformKey::KEY_GAMEPAD_RIGHTSTICK_DOWN,
        "KEY_GAMEPAD_L2", PlatformKey::KEY_GAMEPAD_L2,
        "KEY_GAMEPAD_R2", PlatformKey::KEY_GAMEPAD_R2
        */
    );
}
