#include "SdlEnumTranslation.h"

#include <algorithm>
#include <cassert>

PlatformKey translateSdlKey(SDL_Keycode key) {
    switch (key) {
    case SDLK_F1:               return PlatformKey::KEY_F1;
    case SDLK_F2:               return PlatformKey::KEY_F2;
    case SDLK_F3:               return PlatformKey::KEY_F3;
    case SDLK_F4:               return PlatformKey::KEY_F4;
    case SDLK_F5:               return PlatformKey::KEY_F5;
    case SDLK_F6:               return PlatformKey::KEY_F6;
    case SDLK_F7:               return PlatformKey::KEY_F7;
    case SDLK_F8:               return PlatformKey::KEY_F8;
    case SDLK_F9:               return PlatformKey::KEY_F9;
    case SDLK_F10:              return PlatformKey::KEY_F10;
    case SDLK_F11:              return PlatformKey::KEY_F11;
    case SDLK_F12:              return PlatformKey::KEY_F12;
    case SDLK_F13:              return PlatformKey::KEY_F13;
    case SDLK_F14:              return PlatformKey::KEY_F14;
    case SDLK_F15:              return PlatformKey::KEY_F15;
    case SDLK_F16:              return PlatformKey::KEY_F16;
    case SDLK_F17:              return PlatformKey::KEY_F17;
    case SDLK_F18:              return PlatformKey::KEY_F18;
    case SDLK_F19:              return PlatformKey::KEY_F19;
    case SDLK_F20:              return PlatformKey::KEY_F20;
    case SDLK_F21:              return PlatformKey::KEY_F21;
    case SDLK_F22:              return PlatformKey::KEY_F22;
    case SDLK_F23:              return PlatformKey::KEY_F23;
    case SDLK_F24:              return PlatformKey::KEY_F24;

    case SDLK_1:                return PlatformKey::KEY_DIGIT_1;
    case SDLK_2:                return PlatformKey::KEY_DIGIT_2;
    case SDLK_3:                return PlatformKey::KEY_DIGIT_3;
    case SDLK_4:                return PlatformKey::KEY_DIGIT_4;
    case SDLK_5:                return PlatformKey::KEY_DIGIT_5;
    case SDLK_6:                return PlatformKey::KEY_DIGIT_6;
    case SDLK_7:                return PlatformKey::KEY_DIGIT_7;
    case SDLK_8:                return PlatformKey::KEY_DIGIT_8;
    case SDLK_9:                return PlatformKey::KEY_DIGIT_9;
    case SDLK_0:                return PlatformKey::KEY_DIGIT_0;

    case SDLK_A:                return PlatformKey::KEY_A;
    case SDLK_B:                return PlatformKey::KEY_B;
    case SDLK_C:                return PlatformKey::KEY_C;
    case SDLK_D:                return PlatformKey::KEY_D;
    case SDLK_E:                return PlatformKey::KEY_E;
    case SDLK_F:                return PlatformKey::KEY_F;
    case SDLK_G:                return PlatformKey::KEY_G;
    case SDLK_H:                return PlatformKey::KEY_H;
    case SDLK_I:                return PlatformKey::KEY_I;
    case SDLK_J:                return PlatformKey::KEY_J;
    case SDLK_K:                return PlatformKey::KEY_K;
    case SDLK_L:                return PlatformKey::KEY_L;
    case SDLK_M:                return PlatformKey::KEY_M;
    case SDLK_N:                return PlatformKey::KEY_N;
    case SDLK_O:                return PlatformKey::KEY_O;
    case SDLK_P:                return PlatformKey::KEY_P;
    case SDLK_Q:                return PlatformKey::KEY_Q;
    case SDLK_R:                return PlatformKey::KEY_R;
    case SDLK_S:                return PlatformKey::KEY_S;
    case SDLK_T:                return PlatformKey::KEY_T;
    case SDLK_U:                return PlatformKey::KEY_U;
    case SDLK_V:                return PlatformKey::KEY_V;
    case SDLK_W:                return PlatformKey::KEY_W;
    case SDLK_X:                return PlatformKey::KEY_X;
    case SDLK_Y:                return PlatformKey::KEY_Y;
    case SDLK_Z:                return PlatformKey::KEY_Z;

    case SDLK_RETURN:           return PlatformKey::KEY_RETURN;
    case SDLK_ESCAPE:           return PlatformKey::KEY_ESCAPE;
    case SDLK_TAB:              return PlatformKey::KEY_TAB;
    case SDLK_BACKSPACE:        return PlatformKey::KEY_BACKSPACE;
    case SDLK_SPACE:            return PlatformKey::KEY_SPACE;
    case SDLK_DECIMALSEPARATOR: return PlatformKey::KEY_DECIMAL;
    case SDLK_SEMICOLON:        return PlatformKey::KEY_SEMICOLON;
    case SDLK_PERIOD:           return PlatformKey::KEY_PERIOD;
    case SDLK_SLASH:            return PlatformKey::KEY_SLASH;
    case SDLK_APOSTROPHE:       return PlatformKey::KEY_SINGLEQUOTE;
    case SDLK_BACKSLASH:        return PlatformKey::KEY_BACKSLASH;
    case SDLK_GRAVE:            return PlatformKey::KEY_TILDE;

    case SDLK_KP_MINUS:         return PlatformKey::KEY_SUBTRACT;
    case SDLK_KP_PLUS:          return PlatformKey::KEY_ADD;
    case SDLK_COMMA:            return PlatformKey::KEY_COMMA;
    case SDLK_LEFTBRACKET:      return PlatformKey::KEY_LEFTBRACKET;
    case SDLK_RIGHTBRACKET:     return PlatformKey::KEY_RIGHTBRACKET;

    case SDLK_LEFT:             return PlatformKey::KEY_LEFT;
    case SDLK_RIGHT:            return PlatformKey::KEY_RIGHT;
    case SDLK_UP:               return PlatformKey::KEY_UP;
    case SDLK_DOWN:             return PlatformKey::KEY_DOWN;

    case SDLK_PRINTSCREEN:      return PlatformKey::KEY_PRINTSCREEN;

    case SDLK_INSERT:           return PlatformKey::KEY_INSERT;
    case SDLK_HOME:             return PlatformKey::KEY_HOME;
    case SDLK_END:              return PlatformKey::KEY_END;
    case SDLK_PAGEUP:           return PlatformKey::KEY_PAGEUP;
    case SDLK_PAGEDOWN:         return PlatformKey::KEY_PAGEDOWN;
    case SDLK_DELETE:           return PlatformKey::KEY_DELETE;
    case SDLK_SELECT:           return PlatformKey::KEY_SELECT;

    case SDLK_LCTRL:            return PlatformKey::KEY_CONTROL;
    case SDLK_RCTRL:            return PlatformKey::KEY_CONTROL;
    case SDLK_LALT:             return PlatformKey::KEY_ALT;
    case SDLK_RALT:             return PlatformKey::KEY_ALT;
    case SDLK_LSHIFT:           return PlatformKey::KEY_SHIFT;
    case SDLK_RSHIFT:           return PlatformKey::KEY_SHIFT;

    case SDLK_KP_0:             return PlatformKey::KEY_NUMPAD_0;
    case SDLK_KP_1:             return PlatformKey::KEY_NUMPAD_1;
    case SDLK_KP_2:             return PlatformKey::KEY_NUMPAD_2;
    case SDLK_KP_3:             return PlatformKey::KEY_NUMPAD_3;
    case SDLK_KP_4:             return PlatformKey::KEY_NUMPAD_4;
    case SDLK_KP_5:             return PlatformKey::KEY_NUMPAD_5;
    case SDLK_KP_6:             return PlatformKey::KEY_NUMPAD_6;
    case SDLK_KP_7:             return PlatformKey::KEY_NUMPAD_7;
    case SDLK_KP_8:             return PlatformKey::KEY_NUMPAD_8;
    case SDLK_KP_9:             return PlatformKey::KEY_NUMPAD_9;
    case SDLK_KP_ENTER:         return PlatformKey::KEY_RETURN;

    default:                    return PlatformKey::KEY_NONE;
    }
}

PlatformKey translateSdlGamepadButton(SDL_GamepadButton button) {
    switch (button) {
    case SDL_GAMEPAD_BUTTON_SOUTH:          return PlatformKey::KEY_GAMEPAD_A;
    case SDL_GAMEPAD_BUTTON_EAST:           return PlatformKey::KEY_GAMEPAD_B;
    case SDL_GAMEPAD_BUTTON_WEST:           return PlatformKey::KEY_GAMEPAD_X;
    case SDL_GAMEPAD_BUTTON_NORTH:          return PlatformKey::KEY_GAMEPAD_Y;

    case SDL_GAMEPAD_BUTTON_DPAD_LEFT:      return PlatformKey::KEY_GAMEPAD_LEFT;
    case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:     return PlatformKey::KEY_GAMEPAD_RIGHT;
    case SDL_GAMEPAD_BUTTON_DPAD_UP:        return PlatformKey::KEY_GAMEPAD_UP;
    case SDL_GAMEPAD_BUTTON_DPAD_DOWN:      return PlatformKey::KEY_GAMEPAD_DOWN;

    case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:  return PlatformKey::KEY_GAMEPAD_L1;
    case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: return PlatformKey::KEY_GAMEPAD_R1;
    case SDL_GAMEPAD_BUTTON_LEFT_STICK:     return PlatformKey::KEY_GAMEPAD_L3;
    case SDL_GAMEPAD_BUTTON_RIGHT_STICK:    return PlatformKey::KEY_GAMEPAD_R3;

    case SDL_GAMEPAD_BUTTON_START:          return PlatformKey::KEY_GAMEPAD_START;
    case SDL_GAMEPAD_BUTTON_BACK:           return PlatformKey::KEY_GAMEPAD_BACK;

    case SDL_GAMEPAD_BUTTON_GUIDE:          return PlatformKey::KEY_GAMEPAD_GUIDE;

    // on DS4 touchpad is working like mouse by default and this event treated as left mouse click, so do not expose.
    // case SDL_GAMEPAD_BUTTON_TOUCHPAD:        return PlatformKey::KEY_GAMEPAD_TOUCHPAD;

    default:                                    return PlatformKey::KEY_NONE;
    }
}

PlatformKey translateSdlGamepadAxis(SDL_GamepadAxis axis) {
    switch (axis) {
    case SDL_GAMEPAD_AXIS_LEFTX:            return PlatformKey::KEY_GAMEPAD_LEFTSTICK_RIGHT;
    case SDL_GAMEPAD_AXIS_LEFTY:            return PlatformKey::KEY_GAMEPAD_LEFTSTICK_DOWN;
    case SDL_GAMEPAD_AXIS_RIGHTX:           return PlatformKey::KEY_GAMEPAD_RIGHTSTICK_RIGHT;
    case SDL_GAMEPAD_AXIS_RIGHTY:           return PlatformKey::KEY_GAMEPAD_RIGHTSTICK_DOWN;
    case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:     return PlatformKey::KEY_GAMEPAD_L2;
    case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:    return PlatformKey::KEY_GAMEPAD_R2;
    default:                                return PlatformKey::KEY_NONE;
    }
}

PlatformModifiers translateSdlMods(uint16_t mods) {
    PlatformModifiers result;
    if (static_cast<bool>(mods & SDL_KMOD_SHIFT) ^ static_cast<bool>(mods & SDL_KMOD_CAPS))
        result |= MOD_SHIFT;
    if (mods & SDL_KMOD_CTRL)
        result |= MOD_CTRL;
    if (mods & SDL_KMOD_ALT)
        result |= MOD_ALT;
    if (mods & SDL_KMOD_GUI)
        result |= MOD_META;
    if (mods & SDL_KMOD_NUM)
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

SDL_GLProfile translatePlatformOpenGLProfile(PlatformOpenGLProfile profile) {
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
