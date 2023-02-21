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

void GameKeyboardController::ProcessKeyPressEvent(PlatformKey key) {
    if (isKeyDown_[key])
        return; // Auto repeat

    isKeyDown_[key] = true;
    isKeyDownReported_[key] = false;
}

void GameKeyboardController::ProcessKeyReleaseEvent(PlatformKey key) {
    isKeyDown_[key] = false;
}

void GameKeyboardController::reset() {
    isKeyDown_.fill(false);
    isKeyDownReported_.fill(false);
}
