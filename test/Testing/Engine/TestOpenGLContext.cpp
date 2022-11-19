#include "TestOpenGLContext.h"

#include <utility>

#include "TestStateHandle.h"

TestOpenGLContext::TestOpenGLContext(std::unique_ptr<PlatformOpenGLContext> base, TestStateHandle state) :
    base_(std::move(base)),
    state_(std::move(state))
{}

bool TestOpenGLContext::MakeCurrent() {
    return base_->MakeCurrent();
}

void TestOpenGLContext::SwapBuffers() {
    base_->SwapBuffers();
    state_.YieldExecution();
}

void* TestOpenGLContext::GetProcAddress(const char* name) {
    return base_->GetProcAddress(name);
}
