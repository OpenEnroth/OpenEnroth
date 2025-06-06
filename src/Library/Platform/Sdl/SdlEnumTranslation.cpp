#include "SdlEnumTranslation.h"

#include <algorithm>
#include <cassert>

PlatformKey translateSdlKey(SDL_Scancode key) {
    switch (key) {
    case SDL_SCANCODE_F1:               return PlatformKey::KEY_F1;
    case SDL_SCANCODE_F2:               return PlatformKey::KEY_F2;
    case SDL_SCANCODE_F3:               return PlatformKey::KEY_F3;
    case SDL_SCANCODE_F4:               return PlatformKey::KEY_F4;
    case SDL_SCANCODE_F5:               return PlatformKey::KEY_F5;
    case SDL_SCANCODE_F6:               return PlatformKey::KEY_F6;
    case SDL_SCANCODE_F7:               return PlatformKey::KEY_F7;
    case SDL_SCANCODE_F8:               return PlatformKey::KEY_F8;
    case SDL_SCANCODE_F9:               return PlatformKey::KEY_F9;
    case SDL_SCANCODE_F10:              return PlatformKey::KEY_F10;
    case SDL_SCANCODE_F11:              return PlatformKey::KEY_F11;
    case SDL_SCANCODE_F12:              return PlatformKey::KEY_F12;
    case SDL_SCANCODE_F13:              return PlatformKey::KEY_F13;
    case SDL_SCANCODE_F14:              return PlatformKey::KEY_F14;
    case SDL_SCANCODE_F15:              return PlatformKey::KEY_F15;
    case SDL_SCANCODE_F16:              return PlatformKey::KEY_F16;
    case SDL_SCANCODE_F17:              return PlatformKey::KEY_F17;
    case SDL_SCANCODE_F18:              return PlatformKey::KEY_F18;
    case SDL_SCANCODE_F19:              return PlatformKey::KEY_F19;
    case SDL_SCANCODE_F20:              return PlatformKey::KEY_F20;
    case SDL_SCANCODE_F21:              return PlatformKey::KEY_F21;
    case SDL_SCANCODE_F22:              return PlatformKey::KEY_F22;
    case SDL_SCANCODE_F23:              return PlatformKey::KEY_F23;
    case SDL_SCANCODE_F24:              return PlatformKey::KEY_F24;

    case SDL_SCANCODE_1:                return PlatformKey::KEY_DIGIT_1;
    case SDL_SCANCODE_2:                return PlatformKey::KEY_DIGIT_2;
    case SDL_SCANCODE_3:                return PlatformKey::KEY_DIGIT_3;
    case SDL_SCANCODE_4:                return PlatformKey::KEY_DIGIT_4;
    case SDL_SCANCODE_5:                return PlatformKey::KEY_DIGIT_5;
    case SDL_SCANCODE_6:                return PlatformKey::KEY_DIGIT_6;
    case SDL_SCANCODE_7:                return PlatformKey::KEY_DIGIT_7;
    case SDL_SCANCODE_8:                return PlatformKey::KEY_DIGIT_8;
    case SDL_SCANCODE_9:                return PlatformKey::KEY_DIGIT_9;
    case SDL_SCANCODE_0:                return PlatformKey::KEY_DIGIT_0;

    case SDL_SCANCODE_A:                return PlatformKey::KEY_A;
    case SDL_SCANCODE_B:                return PlatformKey::KEY_B;
    case SDL_SCANCODE_C:                return PlatformKey::KEY_C;
    case SDL_SCANCODE_D:                return PlatformKey::KEY_D;
    case SDL_SCANCODE_E:                return PlatformKey::KEY_E;
    case SDL_SCANCODE_F:                return PlatformKey::KEY_F;
    case SDL_SCANCODE_G:                return PlatformKey::KEY_G;
    case SDL_SCANCODE_H:                return PlatformKey::KEY_H;
    case SDL_SCANCODE_I:                return PlatformKey::KEY_I;
    case SDL_SCANCODE_J:                return PlatformKey::KEY_J;
    case SDL_SCANCODE_K:                return PlatformKey::KEY_K;
    case SDL_SCANCODE_L:                return PlatformKey::KEY_L;
    case SDL_SCANCODE_M:                return PlatformKey::KEY_M;
    case SDL_SCANCODE_N:                return PlatformKey::KEY_N;
    case SDL_SCANCODE_O:                return PlatformKey::KEY_O;
    case SDL_SCANCODE_P:                return PlatformKey::KEY_P;
    case SDL_SCANCODE_Q:                return PlatformKey::KEY_Q;
    case SDL_SCANCODE_R:                return PlatformKey::KEY_R;
    case SDL_SCANCODE_S:                return PlatformKey::KEY_S;
    case SDL_SCANCODE_T:                return PlatformKey::KEY_T;
    case SDL_SCANCODE_U:                return PlatformKey::KEY_U;
    case SDL_SCANCODE_V:                return PlatformKey::KEY_V;
    case SDL_SCANCODE_W:                return PlatformKey::KEY_W;
    case SDL_SCANCODE_X:                return PlatformKey::KEY_X;
    case SDL_SCANCODE_Y:                return PlatformKey::KEY_Y;
    case SDL_SCANCODE_Z:                return PlatformKey::KEY_Z;

    case SDL_SCANCODE_RETURN:           return PlatformKey::KEY_RETURN;
    case SDL_SCANCODE_ESCAPE:           return PlatformKey::KEY_ESCAPE;
    case SDL_SCANCODE_TAB:              return PlatformKey::KEY_TAB;
    case SDL_SCANCODE_BACKSPACE:        return PlatformKey::KEY_BACKSPACE;
    case SDL_SCANCODE_SPACE:            return PlatformKey::KEY_SPACE;
    case SDL_SCANCODE_DECIMALSEPARATOR: return PlatformKey::KEY_DECIMAL;
    case SDL_SCANCODE_SEMICOLON:        return PlatformKey::KEY_SEMICOLON;
    case SDL_SCANCODE_PERIOD:           return PlatformKey::KEY_PERIOD;
    case SDL_SCANCODE_SLASH:            return PlatformKey::KEY_SLASH;
    case SDL_SCANCODE_APOSTROPHE:       return PlatformKey::KEY_SINGLEQUOTE;
    case SDL_SCANCODE_BACKSLASH:        return PlatformKey::KEY_BACKSLASH;
    case SDL_SCANCODE_GRAVE:            return PlatformKey::KEY_TILDE;

    case SDL_SCANCODE_KP_MINUS:         return PlatformKey::KEY_SUBTRACT;
    case SDL_SCANCODE_KP_PLUS:          return PlatformKey::KEY_ADD;
    case SDL_SCANCODE_COMMA:            return PlatformKey::KEY_COMMA;
    case SDL_SCANCODE_LEFTBRACKET:      return PlatformKey::KEY_LEFTBRACKET;
    case SDL_SCANCODE_RIGHTBRACKET:     return PlatformKey::KEY_RIGHTBRACKET;

    case SDL_SCANCODE_LEFT:             return PlatformKey::KEY_LEFT;
    case SDL_SCANCODE_RIGHT:            return PlatformKey::KEY_RIGHT;
    case SDL_SCANCODE_UP:               return PlatformKey::KEY_UP;
    case SDL_SCANCODE_DOWN:             return PlatformKey::KEY_DOWN;

    case SDL_SCANCODE_PRINTSCREEN:      return PlatformKey::KEY_PRINTSCREEN;

    case SDL_SCANCODE_INSERT:           return PlatformKey::KEY_INSERT;
    case SDL_SCANCODE_HOME:             return PlatformKey::KEY_HOME;
    case SDL_SCANCODE_END:              return PlatformKey::KEY_END;
    case SDL_SCANCODE_PAGEUP:           return PlatformKey::KEY_PAGEUP;
    case SDL_SCANCODE_PAGEDOWN:         return PlatformKey::KEY_PAGEDOWN;
    case SDL_SCANCODE_DELETE:           return PlatformKey::KEY_DELETE;
    case SDL_SCANCODE_SELECT:           return PlatformKey::KEY_SELECT;

    case SDL_SCANCODE_LCTRL:            return PlatformKey::KEY_CONTROL;
    case SDL_SCANCODE_RCTRL:            return PlatformKey::KEY_CONTROL;
    case SDL_SCANCODE_LALT:             return PlatformKey::KEY_ALT;
    case SDL_SCANCODE_RALT:             return PlatformKey::KEY_ALT;
    case SDL_SCANCODE_LSHIFT:           return PlatformKey::KEY_SHIFT;
    case SDL_SCANCODE_RSHIFT:           return PlatformKey::KEY_SHIFT;

    case SDL_SCANCODE_KP_0:             return PlatformKey::KEY_NUMPAD_0;
    case SDL_SCANCODE_KP_1:             return PlatformKey::KEY_NUMPAD_1;
    case SDL_SCANCODE_KP_2:             return PlatformKey::KEY_NUMPAD_2;
    case SDL_SCANCODE_KP_3:             return PlatformKey::KEY_NUMPAD_3;
    case SDL_SCANCODE_KP_4:             return PlatformKey::KEY_NUMPAD_4;
    case SDL_SCANCODE_KP_5:             return PlatformKey::KEY_NUMPAD_5;
    case SDL_SCANCODE_KP_6:             return PlatformKey::KEY_NUMPAD_6;
    case SDL_SCANCODE_KP_7:             return PlatformKey::KEY_NUMPAD_7;
    case SDL_SCANCODE_KP_8:             return PlatformKey::KEY_NUMPAD_8;
    case SDL_SCANCODE_KP_9:             return PlatformKey::KEY_NUMPAD_9;
    case SDL_SCANCODE_KP_ENTER:         return PlatformKey::KEY_RETURN;

    default:                            return PlatformKey::KEY_NONE;
    }
}

PlatformKey translateSdlGamepadButton(SDL_GameControllerButton button) {
    switch (button) {
    case SDL_CONTROLLER_BUTTON_A:               return PlatformKey::KEY_GAMEPAD_A;
    case SDL_CONTROLLER_BUTTON_B:               return PlatformKey::KEY_GAMEPAD_B;
    case SDL_CONTROLLER_BUTTON_X:               return PlatformKey::KEY_GAMEPAD_X;
    case SDL_CONTROLLER_BUTTON_Y:               return PlatformKey::KEY_GAMEPAD_Y;

    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:       return PlatformKey::KEY_GAMEPAD_LEFT;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:      return PlatformKey::KEY_GAMEPAD_RIGHT;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:         return PlatformKey::KEY_GAMEPAD_UP;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:       return PlatformKey::KEY_GAMEPAD_DOWN;

    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:    return PlatformKey::KEY_GAMEPAD_L1;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:   return PlatformKey::KEY_GAMEPAD_R1;
    case SDL_CONTROLLER_BUTTON_LEFTSTICK:       return PlatformKey::KEY_GAMEPAD_L3;
    case SDL_CONTROLLER_BUTTON_RIGHTSTICK:      return PlatformKey::KEY_GAMEPAD_R3;

    case SDL_CONTROLLER_BUTTON_START:           return PlatformKey::KEY_GAMEPAD_START;
    case SDL_CONTROLLER_BUTTON_BACK:            return PlatformKey::KEY_GAMEPAD_BACK;

    case SDL_CONTROLLER_BUTTON_GUIDE:           return PlatformKey::KEY_GAMEPAD_GUIDE;

    // on DS4 touchpad is working like mouse by default and this event treated as left mouse click, so not expose
    // case SDL_CONTROLLER_BUTTON_TOUCHPAD:        return PlatformKey::KEY_GAMEPAD_TOUCHPAD;

    default:                                    return PlatformKey::KEY_NONE;
    }
}

PlatformKey translateSdlGamepadAxis(SDL_GameControllerAxis axis) {
    switch (axis) {
    case SDL_CONTROLLER_AXIS_LEFTX:         return PlatformKey::KEY_GAMEPAD_LEFTSTICK_RIGHT;
    case SDL_CONTROLLER_AXIS_LEFTY:         return PlatformKey::KEY_GAMEPAD_LEFTSTICK_DOWN;
    case SDL_CONTROLLER_AXIS_RIGHTX:        return PlatformKey::KEY_GAMEPAD_RIGHTSTICK_RIGHT;
    case SDL_CONTROLLER_AXIS_RIGHTY:        return PlatformKey::KEY_GAMEPAD_RIGHTSTICK_DOWN;
    case SDL_CONTROLLER_AXIS_TRIGGERLEFT:   return PlatformKey::KEY_GAMEPAD_L2;
    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:  return PlatformKey::KEY_GAMEPAD_R2;
    default:                                return PlatformKey::KEY_NONE;
    }
}

PlatformModifiers translateSdlMods(uint16_t mods) {
    PlatformModifiers result;
    if (static_cast<bool>(mods & KMOD_SHIFT) ^ static_cast<bool>(mods & KMOD_CAPS))
        result |= MOD_SHIFT;
    if (mods & KMOD_CTRL)
        result |= MOD_CTRL;
    if (mods & KMOD_ALT)
        result |= MOD_ALT;
    if (mods & KMOD_GUI)
        result |= MOD_META;
    if (mods & KMOD_NUM)
        result |= MOD_NUM;
    return result;
}

PlatformMouseButton translateSdlMouseButton(uint8_t mouseButton) {
    switch (mouseButton) {
    case SDL_BUTTON_LEFT:   return BUTTON_LEFT;
    case SDL_BUTTON_MIDDLE: return BUTTON_MIDDLE;
    case SDL_BUTTON_RIGHT:  return BUTTON_RIGHT;
    default:                return BUTTON_NONE;
    }
}

PlatformMouseButtons translateSdlMouseButtons(uint32_t mouseButtons) {
    PlatformMouseButtons result;
    if (mouseButtons & SDL_BUTTON_LMASK)
        result |= BUTTON_LEFT;
    if (mouseButtons & SDL_BUTTON_MMASK)
        result |= BUTTON_MIDDLE;
    if (mouseButtons & SDL_BUTTON_RMASK)
        result |= BUTTON_RIGHT;
    return result;
}

int translatePlatformVSyncMode(PlatformVSyncMode vsyncMode) {
    switch (vsyncMode) {
    default:                assert(false); [[fallthrough]];
    case GL_VSYNC_NONE:     return 0;
    case GL_VSYNC_ADAPTIVE: return -1;
    case GL_VSYNC_NORMAL:   return 1;
    }
}

SDL_GLprofile translatePlatformOpenGLProfile(PlatformOpenGLProfile profile) {
    switch (profile) {
    default:                        assert(false); [[fallthrough]];
    case GL_PROFILE_CORE:           return SDL_GL_CONTEXT_PROFILE_CORE;
    case GL_PROFILE_COMPATIBILITY:  return SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;
    case GL_PROFILE_ES:             return SDL_GL_CONTEXT_PROFILE_ES;
    }
}

SDL_LogPriority translatePlatformLogLevel(LogLevel logLevel) {
    switch (logLevel) {
    default:            assert(false); [[fallthrough]];
    case LOG_TRACE:     return SDL_LOG_PRIORITY_VERBOSE;
    case LOG_DEBUG:     return SDL_LOG_PRIORITY_DEBUG;
    case LOG_INFO:      return SDL_LOG_PRIORITY_INFO;
    case LOG_WARNING:   return SDL_LOG_PRIORITY_WARN;
    case LOG_ERROR:     return SDL_LOG_PRIORITY_ERROR;
    case LOG_CRITICAL:
    case LOG_NONE:      return SDL_LOG_PRIORITY_CRITICAL; // SDL doesn't have LOG_NONE, so this is the best we can do.
    }
}

LogLevel translateSdlLogLevel(SDL_LogPriority logLevel) {
    switch (logLevel) {
    default:                        assert(false); [[fallthrough]];
    case SDL_LOG_PRIORITY_VERBOSE:  return LOG_TRACE;
    case SDL_LOG_PRIORITY_DEBUG:    return LOG_DEBUG;
    case SDL_LOG_PRIORITY_INFO:     return LOG_INFO;
    case SDL_LOG_PRIORITY_WARN:     return LOG_WARNING;
    case SDL_LOG_PRIORITY_ERROR:    return LOG_ERROR;
    case SDL_LOG_PRIORITY_CRITICAL: return LOG_CRITICAL;
    }
}
