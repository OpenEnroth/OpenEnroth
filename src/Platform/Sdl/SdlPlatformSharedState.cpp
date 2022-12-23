#include "SdlPlatformSharedState.h"

#include <cassert>

#include "Utility/MapAccess.h"

#include "SdlWindow.h"
#include "SdlPlatform.h"
#include "SdlLogger.h"

SdlPlatformSharedState::SdlPlatformSharedState(SdlPlatform *owner): owner_(owner) {
    assert(owner);

    logger_ = std::make_unique<SdlLogger>();
}

SdlPlatformSharedState::~SdlPlatformSharedState() {
    assert(windowById_.empty()); // Platform should be destroyed after all windows.
}

SdlLogger *SdlPlatformSharedState::Logger() const {
    return logger_.get();
}

void SdlPlatformSharedState::LogSdlError(const char *sdlFunctionName) {
    logger_->LogSdlError(sdlFunctionName);
}

void SdlPlatformSharedState::RegisterWindow(SdlWindow *window) {
    assert(!windowById_.contains(window->Id()));
    windowById_[window->Id()] = window;
}

void SdlPlatformSharedState::UnregisterWindow(SdlWindow *window) {
    assert(windowById_.contains(window->Id()));
    windowById_.erase(window->Id());
}

std::vector<uint32_t> SdlPlatformSharedState::AllWindowIds() const {
    std::vector<uint32_t> result;
    for (auto [id, _] : windowById_)
        result.push_back(id);
    return result;
}

SdlWindow *SdlPlatformSharedState::Window(uint32_t id) const {
    assert(windowById_.contains(id));
    return ValueOr(windowById_, id, nullptr);
}
