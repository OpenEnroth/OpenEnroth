#include "TestPlatform.h"

#include <cassert>
#include <utility>

#include "TestWindow.h"
#include "TestEventLoop.h"

TestPlatform::TestPlatform(std::unique_ptr<Platform> base, TestStateHandle state):
    ProxyPlatform(base.get()),
    base_(std::move(base)),
    state_(std::move(state)) {
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

int64_t TestPlatform::TickCount() const {
    return state_->time;
}

void TestPlatform::Reset() {
    state_->time = 0;
}
