#include "KeyboardController.h"

#include "Library/Platform/Interface/PlatformEvents.h"

KeyboardController::KeyboardController() : PlatformEventFilter({EVENT_KEY_PRESS, EVENT_KEY_RELEASE}) {}

bool KeyboardController::IsKeyPressedThisFrame(PlatformKey key) const {
    if (key == PlatformKey::KEY_NONE)
        return false;

    return isKeyPressedThisFrame_[key];
}

bool KeyboardController::IsKeyDown(PlatformKey key) const {
    if (key == PlatformKey::KEY_NONE)
        return false;

    return isKeyDown_[key] || isKeyPressedThisFrame_[key];
}

bool KeyboardController::keyPressEvent(const PlatformKeyEvent *event) {
    if (isKeyDown_[event->key])
        return false; // Auto repeat

    isKeyDown_[event->key] = true;
    isKeyPressedThisFrame_[event->key] = true;
    return false;
}

bool KeyboardController::keyReleaseEvent(const PlatformKeyEvent *event) {
    isKeyDown_[event->key] = false;
    return false;
}

void KeyboardController::processMessages(PlatformEventHandler *eventHandler) {
    isKeyPressedThisFrame_.fill(false);
    ProxyEventLoop::processMessages(eventHandler);
}

void KeyboardController::reset() {
    isKeyDown_.fill(false);
    isKeyPressedThisFrame_.fill(false);
}
