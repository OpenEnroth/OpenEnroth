#include "SdlEnumTranslation.h"

#include <algorithm>
#include <cassert>

PlatformKey TranslateSdlKey(SDL_Scancode key) {
    switch (key) {
    case SDL_SCANCODE_F1:               return PlatformKey::F1;
    case SDL_SCANCODE_F2:               return PlatformKey::F2;
    case SDL_SCANCODE_F3:               return PlatformKey::F3;
    case SDL_SCANCODE_F4:               return PlatformKey::F4;
    case SDL_SCANCODE_F5:               return PlatformKey::F5;
    case SDL_SCANCODE_F6:               return PlatformKey::F6;
    case SDL_SCANCODE_F7:               return PlatformKey::F7;
    case SDL_SCANCODE_F8:               return PlatformKey::F8;
    case SDL_SCANCODE_F9:               return PlatformKey::F9;
    case SDL_SCANCODE_F10:              return PlatformKey::F10;
    case SDL_SCANCODE_F11:              return PlatformKey::F11;
    case SDL_SCANCODE_F12:              return PlatformKey::F12;

    case SDL_SCANCODE_1:                return PlatformKey::Digit1;
    case SDL_SCANCODE_2:                return PlatformKey::Digit2;
    case SDL_SCANCODE_3:                return PlatformKey::Digit3;
    case SDL_SCANCODE_4:                return PlatformKey::Digit4;
    case SDL_SCANCODE_5:                return PlatformKey::Digit5;
    case SDL_SCANCODE_6:                return PlatformKey::Digit6;
    case SDL_SCANCODE_7:                return PlatformKey::Digit7;
    case SDL_SCANCODE_8:                return PlatformKey::Digit8;
    case SDL_SCANCODE_9:                return PlatformKey::Digit9;
    case SDL_SCANCODE_0:                return PlatformKey::Digit0;

    case SDL_SCANCODE_A:                return PlatformKey::A;
    case SDL_SCANCODE_B:                return PlatformKey::B;
    case SDL_SCANCODE_C:                return PlatformKey::C;
    case SDL_SCANCODE_D:                return PlatformKey::D;
    case SDL_SCANCODE_E:                return PlatformKey::E;
    case SDL_SCANCODE_F:                return PlatformKey::F;
    case SDL_SCANCODE_G:                return PlatformKey::G;
    case SDL_SCANCODE_H:                return PlatformKey::H;
    case SDL_SCANCODE_I:                return PlatformKey::I;
    case SDL_SCANCODE_J:                return PlatformKey::J;
    case SDL_SCANCODE_K:                return PlatformKey::K;
    case SDL_SCANCODE_L:                return PlatformKey::L;
    case SDL_SCANCODE_M:                return PlatformKey::M;
    case SDL_SCANCODE_N:                return PlatformKey::N;
    case SDL_SCANCODE_O:                return PlatformKey::O;
    case SDL_SCANCODE_P:                return PlatformKey::P;
    case SDL_SCANCODE_Q:                return PlatformKey::Q;
    case SDL_SCANCODE_R:                return PlatformKey::R;
    case SDL_SCANCODE_S:                return PlatformKey::S;
    case SDL_SCANCODE_T:                return PlatformKey::T;
    case SDL_SCANCODE_U:                return PlatformKey::U;
    case SDL_SCANCODE_V:                return PlatformKey::V;
    case SDL_SCANCODE_W:                return PlatformKey::W;
    case SDL_SCANCODE_X:                return PlatformKey::X;
    case SDL_SCANCODE_Y:                return PlatformKey::Y;
    case SDL_SCANCODE_Z:                return PlatformKey::Z;

    case SDL_SCANCODE_RETURN:           return PlatformKey::Return;
    case SDL_SCANCODE_ESCAPE:           return PlatformKey::Escape;
    case SDL_SCANCODE_TAB:              return PlatformKey::Tab;
    case SDL_SCANCODE_BACKSPACE:        return PlatformKey::Backspace;
    case SDL_SCANCODE_SPACE:            return PlatformKey::Space;
    case SDL_SCANCODE_DECIMALSEPARATOR: return PlatformKey::Decimal;
    case SDL_SCANCODE_SEMICOLON:        return PlatformKey::Semicolon;
    case SDL_SCANCODE_PERIOD:           return PlatformKey::Period;
    case SDL_SCANCODE_SLASH:            return PlatformKey::Slash;
    case SDL_SCANCODE_APOSTROPHE:       return PlatformKey::SingleQuote;
    case SDL_SCANCODE_BACKSLASH:        return PlatformKey::BackSlash;
    case SDL_SCANCODE_GRAVE:            return PlatformKey::Tilde;

    case SDL_SCANCODE_KP_MINUS:         return PlatformKey::Subtract;
    case SDL_SCANCODE_KP_PLUS:          return PlatformKey::Add;
    case SDL_SCANCODE_COMMA:            return PlatformKey::Comma;
    case SDL_SCANCODE_LEFTBRACKET:      return PlatformKey::LeftBracket;
    case SDL_SCANCODE_RIGHTBRACKET:     return PlatformKey::RightBracket;

    case SDL_SCANCODE_LEFT:             return PlatformKey::Left;
    case SDL_SCANCODE_RIGHT:            return PlatformKey::Right;
    case SDL_SCANCODE_UP:               return PlatformKey::Up;
    case SDL_SCANCODE_DOWN:             return PlatformKey::Down;

    case SDL_SCANCODE_PRINTSCREEN:      return PlatformKey::PrintScreen;

    case SDL_SCANCODE_INSERT:           return PlatformKey::Insert;
    case SDL_SCANCODE_HOME:             return PlatformKey::Home;
    case SDL_SCANCODE_END:              return PlatformKey::End;
    case SDL_SCANCODE_PAGEUP:           return PlatformKey::PageUp;
    case SDL_SCANCODE_PAGEDOWN:         return PlatformKey::PageDown;
    case SDL_SCANCODE_DELETE:           return PlatformKey::Delete;
    case SDL_SCANCODE_SELECT:           return PlatformKey::Select;

    case SDL_SCANCODE_LCTRL:            return PlatformKey::Control;
    case SDL_SCANCODE_RCTRL:            return PlatformKey::Control;
    case SDL_SCANCODE_LALT:             return PlatformKey::Alt;
    case SDL_SCANCODE_RALT:             return PlatformKey::Alt;
    case SDL_SCANCODE_LSHIFT:           return PlatformKey::Shift;
    case SDL_SCANCODE_RSHIFT:           return PlatformKey::Shift;

    case SDL_SCANCODE_KP_0:             return PlatformKey::Numpad0;
    case SDL_SCANCODE_KP_1:             return PlatformKey::Numpad1;
    case SDL_SCANCODE_KP_2:             return PlatformKey::Numpad2;
    case SDL_SCANCODE_KP_3:             return PlatformKey::Numpad3;
    case SDL_SCANCODE_KP_4:             return PlatformKey::Numpad4;
    case SDL_SCANCODE_KP_5:             return PlatformKey::Numpad5;
    case SDL_SCANCODE_KP_6:             return PlatformKey::Numpad6;
    case SDL_SCANCODE_KP_7:             return PlatformKey::Numpad7;
    case SDL_SCANCODE_KP_8:             return PlatformKey::Numpad8;
    case SDL_SCANCODE_KP_9:             return PlatformKey::Numpad9;

    default:
        return PlatformKey::None;
    }
}

PlatformKey TranslateSdlGamepadButton(SDL_GameControllerButton button) {
    switch (button) {
    case SDL_CONTROLLER_BUTTON_A:               return PlatformKey::Gamepad_A;
    case SDL_CONTROLLER_BUTTON_B:               return PlatformKey::Gamepad_B;
    case SDL_CONTROLLER_BUTTON_X:               return PlatformKey::Gamepad_X;
    case SDL_CONTROLLER_BUTTON_Y:               return PlatformKey::Gamepad_Y;

    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:       return PlatformKey::Gamepad_Left;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:      return PlatformKey::Gamepad_Right;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:         return PlatformKey::Gamepad_Up;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:       return PlatformKey::Gamepad_Down;

    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:    return PlatformKey::Gamepad_L1;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:   return PlatformKey::Gamepad_R1;
    case SDL_CONTROLLER_BUTTON_LEFTSTICK:       return PlatformKey::Gamepad_L3;
    case SDL_CONTROLLER_BUTTON_RIGHTSTICK:      return PlatformKey::Gamepad_R3;

    case SDL_CONTROLLER_BUTTON_START:           return PlatformKey::Gamepad_Start;
    case SDL_CONTROLLER_BUTTON_BACK:            return PlatformKey::Gamepad_Back;

    case SDL_CONTROLLER_BUTTON_GUIDE:           return PlatformKey::Gamepad_Guide;

    // on DS4 touchpad is working like mouse by default and this event treated as left mouse click, so not expose
    // case SDL_CONTROLLER_BUTTON_TOUCHPAD:        return PlatformKey::Gamepad_Touchpad;

    default:
        return PlatformKey::None;
    }
}

std::pair<PlatformKey, PlatformKeyType> TranslateSdlGamepadAxis(SDL_GameControllerAxis axis, float value) {
    PlatformKey key = PlatformKey::None;
    PlatformKeyType keyType = KEY_TYPE_GAMEPAD_AXIS;

    switch (axis) {
    case SDL_CONTROLLER_AXIS_LEFTX:
        if (value > 0.0f)
            key = PlatformKey::Gamepad_LeftStick_Right;
        else if (value < 0.0f)
            key = PlatformKey::Gamepad_LeftStick_Left;

        break;
    case SDL_CONTROLLER_AXIS_LEFTY:
        if (value > 0.0f)
            key = PlatformKey::Gamepad_LeftStick_Down;
        else if (value < 0.0f)
            key = PlatformKey::Gamepad_LeftStick_Up;

        break;
    case SDL_CONTROLLER_AXIS_RIGHTX:
        if (value > 0.0f)
            key = PlatformKey::Gamepad_RightStick_Right;
        else if (value < 0.0f)
            key = PlatformKey::Gamepad_RightStick_Left;

        break;
    case SDL_CONTROLLER_AXIS_RIGHTY:
        if (value > 0.0f)
            key = PlatformKey::Gamepad_RightStick_Down;
        else if (value < 0.0f)
            key = PlatformKey::Gamepad_RightStick_Up;

        break;
    case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
        key = PlatformKey::Gamepad_L2;
        keyType = KEY_TYPE_GAMEPAD_TRIGGER;
        break;
    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
        key = PlatformKey::Gamepad_R2;
        keyType = KEY_TYPE_GAMEPAD_TRIGGER;
        break;

    default:
        break;
    }

    return {key, keyType};
}

PlatformModifiers TranslateSdlMods(uint16_t mods) {
    PlatformModifiers result;
    if (static_cast<bool>(mods & KMOD_SHIFT) ^ static_cast<bool>(mods & KMOD_CAPS))
        result |= PlatformModifier::Shift;
    if (mods & KMOD_CTRL)
        result |= PlatformModifier::Ctrl;
    if (mods & KMOD_ALT)
        result |= PlatformModifier::Alt;
    if (mods & KMOD_GUI)
        result |= PlatformModifier::Meta;
    if (mods & KMOD_NUM)
        result |= PlatformModifier::NumPad;
    return result;
}

PlatformMouseButton TranslateSdlMouseButton(uint8_t mouseButton) {
    switch (mouseButton) {
    case SDL_BUTTON_LEFT:   return PlatformMouseButton::Left;
    case SDL_BUTTON_MIDDLE: return PlatformMouseButton::Middle;
    case SDL_BUTTON_RIGHT:  return PlatformMouseButton::Right;
    default:                return PlatformMouseButton::None;
    }
}

PlatformMouseButtons TranslateSdlMouseButtons(uint32_t mouseButtons) {
    PlatformMouseButtons result;
    if (mouseButtons & SDL_BUTTON_LMASK)
        result |= PlatformMouseButton::Left;
    if (mouseButtons & SDL_BUTTON_MMASK)
        result |= PlatformMouseButton::Middle;
    if (mouseButtons & SDL_BUTTON_RMASK)
        result |= PlatformMouseButton::Right;
    return result;
}

int TranslatePlatformVSyncMode(PlatformVSyncMode vsyncMode) {
    switch (vsyncMode) {
    case GL_VSYNC_NONE:
        return 0;
    case GL_VSYNC_ADAPTIVE:
        return -1;
    case GL_VSYNC_NORMAL:
        return 1;
    }

    assert(false);
    return 0; // Make the compiler happy.
}

SDL_GLprofile TranslatePlatformOpenGLProfile(PlatformOpenGLProfile profile) {
    switch (profile) {
    case GL_PROFILE_CORE:
        return SDL_GL_CONTEXT_PROFILE_CORE;
    case GL_PROFILE_COMPATIBILITY:
        return SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;
    case GL_PROFILE_ES:
        return SDL_GL_CONTEXT_PROFILE_ES;
    }

    assert(false);
    return SDL_GL_CONTEXT_PROFILE_CORE; // Make the compiler happy.
}

SDL_LogPriority TranslatePlatformLogLevel(PlatformLogLevel logLevel) {
    switch (logLevel) {
    case LogVerbose:    return SDL_LOG_PRIORITY_VERBOSE;
    case LogDebug:      return SDL_LOG_PRIORITY_DEBUG;
    case LogInfo:       return SDL_LOG_PRIORITY_INFO;
    case LogWarning:    return SDL_LOG_PRIORITY_WARN;
    case LogError:      return SDL_LOG_PRIORITY_ERROR;
    case LogCritical:   return SDL_LOG_PRIORITY_CRITICAL;
    default:
        assert(false);
        return SDL_LOG_PRIORITY_VERBOSE;
    }
}

PlatformLogLevel TranslateSdlLogLevel(SDL_LogPriority logLevel) {
    switch (logLevel) {
    case SDL_LOG_PRIORITY_VERBOSE:  return LogVerbose;
    case SDL_LOG_PRIORITY_DEBUG:    return LogDebug;
    case SDL_LOG_PRIORITY_INFO:     return LogInfo;
    case SDL_LOG_PRIORITY_WARN:     return LogWarning;
    case SDL_LOG_PRIORITY_ERROR:    return LogError;
    case SDL_LOG_PRIORITY_CRITICAL: return LogCritical;
    default:
        assert(false);
        return LogVerbose;
    }
}
