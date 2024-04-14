#include "UiEventHandler.h"

#include "KeyPressEventHandler.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <Engine/Graphics/Renderer/Renderer.h>
#include <Engine/Engine.h>

#include <algorithm>
#include <memory>

UiEventHandler::UiEventHandler(Renderer &renderer, const GetMainContextFunc &getMainContext)
    : PlatformEventFilter(EVENTS_ALL)
    , _getMainContext(getMainContext)
    , _renderer(renderer) {
}

void UiEventHandler::addKeyPressEventHandler(PlatformKey platformKey, const std::function<void()> &callback) {
    _keyPressEventHandlers.push_back(std::make_unique<KeyPressEventHandler>(platformKey, callback));
}

bool UiEventHandler::keyPressEvent(const PlatformKeyEvent *event) {
    Rml::Context *context = _getMainContext();
    bool result = context->ProcessKeyDown(convertKey(event->key), getKeyModifierState());
    if (event->key == PlatformKey::KEY_RETURN) {
        result &= context->ProcessTextInput('\n');
    }

    for (auto &&eventHandler : _keyPressEventHandlers) {
        if (eventHandler->keyPressEvent(event)) {
            return true;
        }
    }
    return !result;
}

bool UiEventHandler::keyReleaseEvent(const PlatformKeyEvent *event) {
    return !_getMainContext()->ProcessKeyDown(convertKey(event->key), getKeyModifierState());
}

bool UiEventHandler::mouseMoveEvent(const PlatformMouseEvent *event) {
    auto point = _renderer.mapScreenPointToRender(event->pos);
    return !_getMainContext()->ProcessMouseMove(point.x, point.y, getKeyModifierState());
}

bool UiEventHandler::mousePressEvent(const PlatformMouseEvent *event) {
    return !_getMainContext()->ProcessMouseButtonDown(convertMouseButton(event->button), getKeyModifierState());
}

bool UiEventHandler::mouseReleaseEvent(const PlatformMouseEvent *event) {
    return !_getMainContext()->ProcessMouseButtonUp(convertMouseButton(event->button), getKeyModifierState());
}

bool UiEventHandler::wheelEvent(const PlatformWheelEvent *event) {
    return !_getMainContext()->ProcessMouseWheel(Rml::Vector2f(event->angleDelta.x, event->angleDelta.y), getKeyModifierState());
}

bool UiEventHandler::textInputEvent(const PlatformTextInputEvent *event) {
    return !_getMainContext()->ProcessTextInput(event->text);
}

bool UiEventHandler::resizeEvent(const PlatformResizeEvent *event) {
    Rml::ReleaseTextures();
    return false;
}

bool UiEventHandler::activationEvent(const PlatformWindowEvent *event) {
    Rml::ReleaseTextures();
    return false;
}

Rml::Input::KeyIdentifier UiEventHandler::convertKey(PlatformKey key) {
    switch (key) {
    case PlatformKey::KEY_NONE:         return Rml::Input::KI_UNKNOWN;
    case PlatformKey::KEY_ESCAPE:       return Rml::Input::KI_ESCAPE;
    case PlatformKey::KEY_SPACE:        return Rml::Input::KI_SPACE;
    case PlatformKey::KEY_DIGIT_0:      return Rml::Input::KI_0;
    case PlatformKey::KEY_DIGIT_1:      return Rml::Input::KI_1;
    case PlatformKey::KEY_DIGIT_2:      return Rml::Input::KI_2;
    case PlatformKey::KEY_DIGIT_3:      return Rml::Input::KI_3;
    case PlatformKey::KEY_DIGIT_4:      return Rml::Input::KI_4;
    case PlatformKey::KEY_DIGIT_5:      return Rml::Input::KI_5;
    case PlatformKey::KEY_DIGIT_6:      return Rml::Input::KI_6;
    case PlatformKey::KEY_DIGIT_7:      return Rml::Input::KI_7;
    case PlatformKey::KEY_DIGIT_8:      return Rml::Input::KI_8;
    case PlatformKey::KEY_DIGIT_9:      return Rml::Input::KI_9;
    case PlatformKey::KEY_A:            return Rml::Input::KI_A;
    case PlatformKey::KEY_B:            return Rml::Input::KI_B;
    case PlatformKey::KEY_C:            return Rml::Input::KI_C;
    case PlatformKey::KEY_D:            return Rml::Input::KI_D;
    case PlatformKey::KEY_E:            return Rml::Input::KI_E;
    case PlatformKey::KEY_F:            return Rml::Input::KI_F;
    case PlatformKey::KEY_G:            return Rml::Input::KI_G;
    case PlatformKey::KEY_H:            return Rml::Input::KI_H;
    case PlatformKey::KEY_I:            return Rml::Input::KI_I;
    case PlatformKey::KEY_J:            return Rml::Input::KI_J;
    case PlatformKey::KEY_K:            return Rml::Input::KI_K;
    case PlatformKey::KEY_L:            return Rml::Input::KI_L;
    case PlatformKey::KEY_M:            return Rml::Input::KI_M;
    case PlatformKey::KEY_N:            return Rml::Input::KI_N;
    case PlatformKey::KEY_O:            return Rml::Input::KI_O;
    case PlatformKey::KEY_P:            return Rml::Input::KI_P;
    case PlatformKey::KEY_Q:            return Rml::Input::KI_Q;
    case PlatformKey::KEY_R:            return Rml::Input::KI_R;
    case PlatformKey::KEY_S:            return Rml::Input::KI_S;
    case PlatformKey::KEY_T:            return Rml::Input::KI_T;
    case PlatformKey::KEY_U:            return Rml::Input::KI_U;
    case PlatformKey::KEY_V:            return Rml::Input::KI_V;
    case PlatformKey::KEY_W:            return Rml::Input::KI_W;
    case PlatformKey::KEY_X:            return Rml::Input::KI_X;
    case PlatformKey::KEY_Y:            return Rml::Input::KI_Y;
    case PlatformKey::KEY_Z:            return Rml::Input::KI_Z;
    case PlatformKey::KEY_SEMICOLON:    return Rml::Input::KI_OEM_1;
    case PlatformKey::KEY_ADD:          return Rml::Input::KI_OEM_PLUS;
    case PlatformKey::KEY_COMMA:        return Rml::Input::KI_OEM_COMMA;
    case PlatformKey::KEY_MINUS:        return Rml::Input::KI_OEM_MINUS;
    case PlatformKey::KEY_PERIOD:       return Rml::Input::KI_OEM_PERIOD;
    case PlatformKey::KEY_SLASH:        return Rml::Input::KI_OEM_2;
    case PlatformKey::KEY_TILDE:        return Rml::Input::KI_OEM_3;
    case PlatformKey::KEY_LEFTBRACKET:  return Rml::Input::KI_OEM_4;
    case PlatformKey::KEY_BACKSLASH:    return Rml::Input::KI_OEM_5;
    case PlatformKey::KEY_RIGHTBRACKET: return Rml::Input::KI_OEM_6;
    case PlatformKey::KEY_NUMPAD_0:     return Rml::Input::KI_NUMPAD0;
    case PlatformKey::KEY_NUMPAD_1:     return Rml::Input::KI_NUMPAD1;
    case PlatformKey::KEY_NUMPAD_2:     return Rml::Input::KI_NUMPAD2;
    case PlatformKey::KEY_NUMPAD_3:     return Rml::Input::KI_NUMPAD3;
    case PlatformKey::KEY_NUMPAD_4:     return Rml::Input::KI_NUMPAD4;
    case PlatformKey::KEY_NUMPAD_5:     return Rml::Input::KI_NUMPAD5;
    case PlatformKey::KEY_NUMPAD_6:     return Rml::Input::KI_NUMPAD6;
    case PlatformKey::KEY_NUMPAD_7:     return Rml::Input::KI_NUMPAD7;
    case PlatformKey::KEY_NUMPAD_8:     return Rml::Input::KI_NUMPAD8;
    case PlatformKey::KEY_NUMPAD_9:     return Rml::Input::KI_NUMPAD9;
    case PlatformKey::KEY_MULTIPLY:     return Rml::Input::KI_MULTIPLY;
    case PlatformKey::KEY_SUBTRACT:     return Rml::Input::KI_SUBTRACT;
    case PlatformKey::KEY_DIVIDE:       return Rml::Input::KI_DIVIDE;
    case PlatformKey::KEY_EQUALS:       return Rml::Input::KI_OEM_NEC_EQUAL;
    case PlatformKey::KEY_BACKSPACE:    return Rml::Input::KI_BACK;
    case PlatformKey::KEY_TAB:          return Rml::Input::KI_TAB;
    case PlatformKey::KEY_CLEAR:        return Rml::Input::KI_CLEAR;
    case PlatformKey::KEY_RETURN:       return Rml::Input::KI_RETURN;
    case PlatformKey::KEY_PAUSE:        return Rml::Input::KI_PAUSE;
    case PlatformKey::KEY_CAPSLOCK:     return Rml::Input::KI_CAPITAL;
    case PlatformKey::KEY_PAGEUP:       return Rml::Input::KI_PRIOR;
    case PlatformKey::KEY_PAGEDOWN:     return Rml::Input::KI_NEXT;
    case PlatformKey::KEY_END:          return Rml::Input::KI_END;
    case PlatformKey::KEY_HOME:         return Rml::Input::KI_HOME;
    case PlatformKey::KEY_LEFT:         return Rml::Input::KI_LEFT;
    case PlatformKey::KEY_UP:           return Rml::Input::KI_UP;
    case PlatformKey::KEY_RIGHT:        return Rml::Input::KI_RIGHT;
    case PlatformKey::KEY_DOWN:         return Rml::Input::KI_DOWN;
    case PlatformKey::KEY_INSERT:       return Rml::Input::KI_INSERT;
    case PlatformKey::KEY_DELETE:       return Rml::Input::KI_DELETE;
    case PlatformKey::KEY_HELP:         return Rml::Input::KI_HELP;
    case PlatformKey::KEY_F1:           return Rml::Input::KI_F1;
    case PlatformKey::KEY_F2:           return Rml::Input::KI_F2;
    case PlatformKey::KEY_F3:           return Rml::Input::KI_F3;
    case PlatformKey::KEY_F4:           return Rml::Input::KI_F4;
    case PlatformKey::KEY_F5:           return Rml::Input::KI_F5;
    case PlatformKey::KEY_F6:           return Rml::Input::KI_F6;
    case PlatformKey::KEY_F7:           return Rml::Input::KI_F7;
    case PlatformKey::KEY_F8:           return Rml::Input::KI_F8;
    case PlatformKey::KEY_F9:           return Rml::Input::KI_F9;
    case PlatformKey::KEY_F10:          return Rml::Input::KI_F10;
    case PlatformKey::KEY_F11:          return Rml::Input::KI_F11;
    case PlatformKey::KEY_F12:          return Rml::Input::KI_F12;
    case PlatformKey::KEY_F13:          return Rml::Input::KI_F13;
    case PlatformKey::KEY_F14:          return Rml::Input::KI_F14;
    case PlatformKey::KEY_F15:          return Rml::Input::KI_F15;
    case PlatformKey::KEY_NUMLOCKCLEAR: return Rml::Input::KI_NUMLOCK;
    case PlatformKey::KEY_SCROLLLOCK:   return Rml::Input::KI_SCROLL;
    case PlatformKey::KEY_SHIFT:        return Rml::Input::KI_LSHIFT; //we're currently not discerning between left and right shift
    case PlatformKey::KEY_CONTROL:      return Rml::Input::KI_LCONTROL; //we're currently not discerning between left and right ctrl
    case PlatformKey::KEY_ALT:          return Rml::Input::KI_LMENU; //we're currently not discerning between left and right alt
    default: break;
    }

    return Rml::Input::KI_UNKNOWN;
}

int UiEventHandler::convertMouseButton(PlatformMouseButton button) {
    switch (button) {
    case PlatformMouseButton::BUTTON_LEFT: return 0;
    case PlatformMouseButton::BUTTON_RIGHT: return 1;
    case PlatformMouseButton::BUTTON_MIDDLE: return 2;
    default: return 3;
    }
}

int UiEventHandler::getKeyModifierState() {
    return 0;

    /*
    * In theory we could look at these mod state from SDL but since we want to be backend agnostic
    * I'll return 0 for now and keep this example with SDL as a good reference
    *
    SDL_Keymod sdl_mods = SDL_GetModState();

    int retval = 0;

    if (sdl_mods & KMOD_CTRL)
        retval |= Rml::Input::KM_CTRL;

    if (sdl_mods & KMOD_SHIFT)
        retval |= Rml::Input::KM_SHIFT;

    if (sdl_mods & KMOD_ALT)
        retval |= Rml::Input::KM_ALT;

    if (sdl_mods & KMOD_NUM)
        retval |= Rml::Input::KM_NUMLOCK;

    if (sdl_mods & KMOD_CAPS)
        retval |= Rml::Input::KM_CAPSLOCK;

    return retval;
    */
}
