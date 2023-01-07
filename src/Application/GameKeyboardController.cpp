#include "GameKeyboardController.h"

#include "Platform/PlatformEvents.h"

GameKeyboardController::GameKeyboardController() {}

bool GameKeyboardController::ConsumeKeyPress(PlatformKey key) {
    if (key == PlatformKey::None)
        return false;

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

void GameKeyboardController::ProcessKeyPressEvent(const PlatformKeyEvent *event) {
    if (isKeyDown_[event->key])
        return; // Auto repeat

    isKeyDown_[event->key] = true;
    isKeyDownReported_[event->key] = false;
}

void GameKeyboardController::ProcessKeyReleaseEvent(const PlatformKeyEvent *event) {
    isKeyDown_[event->key] = false;
}
