#include "KeyboardController.h"

#include "Library/Platform/Interface/PlatformEvents.h"

KeyboardController::KeyboardController() : PlatformEventFilter({EVENT_KEY_PRESS, EVENT_KEY_RELEASE}) {}

bool KeyboardController::ConsumeKeyPress(PlatformKey key) {
    if (key == PlatformKey::KEY_NONE)
        return false;

    // Note that we're not checking for isKeyDown_[key] here, and this is intentional. This becomes relevant if both
    // key press and key release events happen inside a single frame, and thus even though the player has pressed the
    // key, it's not held down when we get around to actually handling it.

    if (!isKeyDownReportPending_[key])
        return false;

    isKeyDownReportPending_[key] = false;
    return true;
}

bool KeyboardController::IsKeyDown(PlatformKey key) const {
    if (key == PlatformKey::KEY_NONE)
        return false;

    return isKeyDown_[key] || isKeyDownThisFrame_[key];
}

bool KeyboardController::keyPressEvent(const PlatformKeyEvent *event) {
    if (isKeyDown_[event->key])
        return false; // Auto repeat

    isKeyDown_[event->key] = true;
    isKeyDownThisFrame_[event->key] = true;
    isKeyDownReportPending_[event->key] = true;
    return false;
}

bool KeyboardController::keyReleaseEvent(const PlatformKeyEvent *event) {
    isKeyDown_[event->key] = false;
    return false;
}

void KeyboardController::processMessages(PlatformEventHandler *eventHandler) {
    isKeyDownThisFrame_.fill(false);
    ProxyEventLoop::processMessages(eventHandler);
}

void KeyboardController::reset() {
    isKeyDown_.fill(false);
    isKeyDownReportPending_.fill(false);
}
