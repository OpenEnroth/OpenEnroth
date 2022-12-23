#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>

class SdlWindow;
class SdlPlatform;
class SdlLogger;
class PlatformEvent;
class PlatformEventHandler;

class SdlPlatformSharedState {
 public:
    explicit SdlPlatformSharedState(SdlPlatform *owner);
    ~SdlPlatformSharedState();

    SdlLogger *Logger() const;
    void LogSdlError(const char *sdlFunctionName);

    void RegisterWindow(SdlWindow *window);
    void UnregisterWindow(SdlWindow *window);
    std::vector<uint32_t> AllWindowIds() const;
    SdlWindow *Window(uint32_t id) const;

 private:
    SdlPlatform *owner_ = nullptr;
    std::unique_ptr<SdlLogger> logger_;
    std::unordered_map<uint32_t, SdlWindow *> windowById_;
};
