#include "SdlPlatformSharedState.h"

#include <cassert>

#include "Utility/Log.h"
#include "Utility/MapAccess.h"

#include "SdlWindow.h"
#include "SdlPlatform.h"

SdlPlatformSharedState::SdlPlatformSharedState(SdlPlatform *owner, Log *log): owner_(owner), log_(log) {
    assert(owner);
    assert(log);
}

SdlPlatformSharedState::~SdlPlatformSharedState() {
    assert(windowById_.empty()); // Platform should be destroyed after all windows.
}

void SdlPlatformSharedState::LogSdlError(const char *sdlFunctionName) {
    log_->Warning("SDL error in %s: %s.", sdlFunctionName, SDL_GetError());
}

void SdlPlatformSharedState::RegisterWindow(SdlWindow *window) {
    assert(!windowById_.contains(window->Id()));
    windowById_[window->Id()] = window;
}

void SdlPlatformSharedState::UnregisterWindow(SdlWindow *window) {
    assert(windowById_.contains(window->Id()));
    windowById_.erase(window->Id());
}

SdlWindow *SdlPlatformSharedState::Window(uint32_t id) const {
    assert(windowById_.contains(id));
    return ValueOr(windowById_, id, nullptr);
}
