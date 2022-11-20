#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>


class Log;
class SdlWindow;
class SdlPlatform;
class PlatformEvent;
class PlatformEventHandler;

class SdlPlatformSharedState {
 public:
    SdlPlatformSharedState(SdlPlatform *owner, Log *log);
    ~SdlPlatformSharedState();

    void LogSdlError(const char *sdlFunctionName);

    void RegisterWindow(SdlWindow *window);
    void UnregisterWindow(SdlWindow *window);
    std::vector<uint32_t> AllWindowIds() const;
    SdlWindow *Window(uint32_t id) const;

 private:
    SdlPlatform *owner_ = nullptr;
    Log *log_ = nullptr;
    std::unordered_map<uint32_t, SdlWindow *> windowById_;
};
