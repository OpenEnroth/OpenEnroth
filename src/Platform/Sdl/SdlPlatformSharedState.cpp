#include "SdlPlatformSharedState.h"

#include "Utility/Log.h"
#include "Utility/MapAccess.h"

#include "SdlWindow.h"
#include "SdlPlatform.h"

SdlPlatformSharedState::SdlPlatformSharedState(SdlPlatform *owner, Log *log): owner_(owner), log_(log) {}

void SdlPlatformSharedState::LogSdlError(const char *sdlFunctionName) {
    log_->Warning("SDL error in %s: %s.", sdlFunctionName, SDL_GetError());
}

void SdlPlatformSharedState::RegisterWindow(SdlWindow *window) {
    assert(!windowInfoById_.contains(window->Id()));
    windowInfoById_[window->Id()] = window;
}

void SdlPlatformSharedState::UnregisterWindow(SdlWindow *window) {
    assert(windowInfoById_.contains(window->Id()));

    windowInfoById_.erase(window->Id());
}

void SdlPlatformSharedState::SendEvent(uint32_t windowId, PlatformEvent *event) {
    assert(windowInfoById_.contains(windowId));

    owner_->SendEvent(windowInfoById_[windowId], event); // Send it through the platform properly.
}
