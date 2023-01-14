#include "TestWindow.h"

#include <cassert>
#include <utility>

#include "TestOpenGLContext.h"

TestWindow::TestWindow(std::unique_ptr<PlatformWindow> base, TestStateHandle state) :
    ProxyWindow(base.get()),
    base_(std::move(base)),
    state_(std::move(state)) {
    assert(state_->window == nullptr);
    state_->window = this;
}

TestWindow::~TestWindow() {
    assert(state_->window == this);
    state_->window = nullptr;
}

std::unique_ptr<PlatformOpenGLContext> TestWindow::CreateOpenGLContext(const PlatformOpenGLOptions &options) {
    return std::make_unique<TestOpenGLContext>(base_->CreateOpenGLContext(options), state_);
}
