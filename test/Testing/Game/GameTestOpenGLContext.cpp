#include "GameTestOpenGLContext.h"

#include <utility>

#include "GameTestStateHandle.h"

GameTestOpenGLContext::GameTestOpenGLContext(std::unique_ptr<PlatformOpenGLContext> base, GameTestStateHandle state) :
    base_(std::move(base)),
    state_(std::move(state))
{}

bool GameTestOpenGLContext::MakeCurrent() {
    return base_->MakeCurrent();
}

void GameTestOpenGLContext::SwapBuffers() {
    base_->SwapBuffers();
    state_.SwitchSide();
}

void* GameTestOpenGLContext::GetProcAddress(const char* name) {
    return base_->GetProcAddress(name);
}
