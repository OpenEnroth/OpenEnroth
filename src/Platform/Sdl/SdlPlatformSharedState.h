#pragma once

#include <cstdint>
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
    SdlWindow *Window(uint32_t id) const;

    bool IsTerminating() const {
        return terminating_;
    }

    void SetTerminating(bool terminating) {
        terminating_ = terminating;
    }

 private:
    SdlPlatform *owner_ = nullptr;
    Log *log_ = nullptr;
    bool terminating_ = false;
    std::unordered_map<uint32_t, SdlWindow *> windowById_;
};
