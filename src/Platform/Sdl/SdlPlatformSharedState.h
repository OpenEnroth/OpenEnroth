#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>

class SdlWindow;
class SdlPlatform;
class PlatformEvent;
class PlatformEventHandler;
class PlatformLogger;

class SdlPlatformSharedState {
 public:
    SdlPlatformSharedState(SdlPlatform *owner, PlatformLogger *logger);
    ~SdlPlatformSharedState();

    void LogSdlError(const char *sdlFunctionName);

    void RegisterWindow(SdlWindow *window);
    void UnregisterWindow(SdlWindow *window);
    std::vector<uint32_t> AllWindowIds() const;
    SdlWindow *Window(uint32_t id) const;

 private:
    SdlPlatform *owner_ = nullptr;
    PlatformLogger *logger_ = nullptr;
    std::unordered_map<uint32_t, SdlWindow *> windowById_;
};
