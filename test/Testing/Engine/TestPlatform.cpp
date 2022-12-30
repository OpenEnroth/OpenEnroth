#include "TestPlatform.h"

#include <cassert>
#include <utility>

#include "TestWindow.h"
#include "TestEventLoop.h"

TestPlatform::TestPlatform(std::unique_ptr<Platform> base, TestStateHandle state):
    base_(std::move(base)),
    state_(std::move(state))
{
    assert(state_->platform == nullptr);
    state_->platform = this;
}

TestPlatform::~TestPlatform() {
    assert(state_->platform == this);
    state_->platform = nullptr;
}

std::unique_ptr<PlatformWindow> TestPlatform::CreateWindow() {
    return std::make_unique<TestWindow>(base_->CreateWindow(), state_);
}

std::unique_ptr<PlatformEventLoop> TestPlatform::CreateEventLoop() {
    return std::make_unique<TestEventLoop>(base_->CreateEventLoop(), state_);
}

void TestPlatform::SetCursorShown(bool cursorShown) {
    return base_->SetCursorShown(cursorShown);
}

bool TestPlatform::IsCursorShown() const {
    return base_->IsCursorShown();
}

std::vector<Recti> TestPlatform::DisplayGeometries() const {
    return base_->DisplayGeometries();
}

void TestPlatform::ShowMessageBox(const std::string &message, const std::string& title) const {
    base_->ShowMessageBox(message, title);
}

int64_t TestPlatform::TickCount() const {
    return state_->time;
}

std::string TestPlatform::WinQueryRegistry(const std::string &path) const {
    return base_->WinQueryRegistry(path);
}

void TestPlatform::Reset() {
    state_->time = 0;
}
