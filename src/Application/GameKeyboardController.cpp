#include "GameKeyboardController.h"

#include "Library/Platform/Interface/PlatformEvents.h"

GameKeyboardController::GameKeyboardController() : PlatformEventFilter({EVENT_KEY_PRESS, EVENT_KEY_RELEASE}) {}

bool GameKeyboardController::ConsumeKeyPress(PlatformKey key) {
    if (key == PlatformKey::KEY_NONE)
        return false;

    // TODO(captainurist): this is false if we have received press & release events inside a single frame.
    if (!isKeyDown_[key])
        return false;

    if (isKeyDownReported_[key])
        return false;

    isKeyDownReported_[key] = true;
    return true;
}

bool GameKeyboardController::IsKeyDown(PlatformKey key) const {
    return isKeyDown_[key];
}

bool GameKeyboardController::keyPressEvent(const PlatformKeyEvent *event) {
    if (isKeyDown_[event->key])
        return false; // Auto repeat

    isKeyDown_[event->key] = true;
    isKeyDownReported_[event->key] = false;
    return false;
}

bool GameKeyboardController::keyReleaseEvent(const PlatformKeyEvent *event) {
    isKeyDown_[event->key] = false;
    return false;
}

void GameKeyboardController::reset() {
    isKeyDown_.fill(false);
    isKeyDownReported_.fill(false);
}
