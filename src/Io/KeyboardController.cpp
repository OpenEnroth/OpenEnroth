#include "KeyboardController.h"

#include "Library/Platform/Interface/PlatformEvents.h"

KeyboardController::KeyboardController() : PlatformEventFilter({EVENT_KEY_PRESS, EVENT_KEY_RELEASE}) {}

bool KeyboardController::isKeyPressedThisFrame(PlatformKey key) const {
    if (key == PlatformKey::KEY_NONE)
        return false;

    return _isKeyPressedThisFrame[key];
}

bool KeyboardController::isKeyDownThisFrame(PlatformKey key) const {
    if (key == PlatformKey::KEY_NONE)
        return false;

    return _isKeyDown[key] || _isKeyPressedThisFrame[key];
}

bool KeyboardController::keyPressEvent(const PlatformKeyEvent *event) {
    if (_isKeyDown[event->key])
        return false; // Auto repeat

    _isKeyDown[event->key] = true;
    _isKeyPressedThisFrame[event->key] = true;
    return false;
}

bool KeyboardController::keyReleaseEvent(const PlatformKeyEvent *event) {
    _isKeyDown[event->key] = false;
    return false;
}

void KeyboardController::processMessages(PlatformEventHandler *eventHandler) {
    _isKeyPressedThisFrame.fill(false);
    ProxyEventLoop::processMessages(eventHandler);
}

void KeyboardController::reset() {
    _isKeyDown.fill(false);
    _isKeyPressedThisFrame.fill(false);
}
