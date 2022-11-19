#include "GameTestWindow.h"

#include <cassert>
#include <utility>

#include "GameTestOpenGLContext.h"

GameTestWindow::GameTestWindow(std::unique_ptr<PlatformWindow> base, GameTestStateHandle state) :
    base_(std::move(base)),
    state_(std::move(state)) {
    assert(state_->window == nullptr);
    state_->window = this;
}

GameTestWindow::~GameTestWindow() {
    assert(state_->window == this);
    state_->window = nullptr;
}

void GameTestWindow::SetTitle(const std::string &title) {
    base_->SetTitle(title);
}

std::string GameTestWindow::Title() const {
    return base_->Title();
}

void GameTestWindow::Resize(const Sizei &size) {
    base_->Resize(size);
}

Sizei GameTestWindow::Size() const {
    return base_->Size();
}

void GameTestWindow::SetPosition(const Pointi &pos) {
    base_->SetPosition(pos);
}

Pointi GameTestWindow::Position() const {
    return base_->Position();
}

void GameTestWindow::SetVisible(bool visible) {
    base_->SetVisible(visible);
}

bool GameTestWindow::IsVisible() const {
    return base_->IsVisible();
}

void GameTestWindow::SetFullscreen(bool fullscreen) {
    base_->SetFullscreen(fullscreen);
}

bool GameTestWindow::IsFullscreen() const {
    return base_->IsFullscreen();
}

void GameTestWindow::SetBorderless(bool borderless) {
    base_->SetBorderless(borderless);
}

bool GameTestWindow::IsBorderless() const {
    return base_->IsBorderless();
}

void GameTestWindow::SetGrabsMouse(bool grabsMouse) {
    base_->SetGrabsMouse(grabsMouse);
}

bool GameTestWindow::GrabsMouse() const {
    return base_->GrabsMouse();
}

void GameTestWindow::Activate() {
    base_->Activate();
}

uintptr_t GameTestWindow::SystemHandle() const {
    return base_->SystemHandle();
}

std::unique_ptr<PlatformOpenGLContext> GameTestWindow::CreateOpenGLContext(const PlatformOpenGLOptions &options) {
    return std::make_unique<GameTestOpenGLContext>(base_->CreateOpenGLContext(options), state_);
}
