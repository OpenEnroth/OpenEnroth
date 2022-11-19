#include "GameTestPlatform.h"

#include <utility>

#include "GameTestWindow.h"
#include "GameTestEventLoop.h"

GameTestPlatform::GameTestPlatform(std::unique_ptr<Platform> base, GameTestStateHandle state):
    base_(std::move(base)),
    state_(std::move(state))
{}

std::unique_ptr<PlatformWindow> GameTestPlatform::CreateWindow() {
    return std::make_unique<GameTestWindow>(base_->CreateWindow(), state_);
}

std::unique_ptr<PlatformEventLoop> GameTestPlatform::CreateEventLoop() {
    return std::make_unique<GameTestEventLoop>(base_->CreateEventLoop(), state_);
}

void GameTestPlatform::SetCursorShown(bool cursorShown) {
    return base_->SetCursorShown(cursorShown);
}

bool GameTestPlatform::IsCursorShown() const {
    return base_->IsCursorShown();
}

std::vector<Recti> GameTestPlatform::DisplayGeometries() const {
    return base_->DisplayGeometries();
}
