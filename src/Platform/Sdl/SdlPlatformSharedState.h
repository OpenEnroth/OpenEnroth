#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>

class SdlWindow;
class SdlPlatform;
class SdlGamepad;
class PlatformEvent;
class PlatformEventHandler;
class PlatformLogger;
class PlatformGamepad;

class SdlPlatformSharedState {
 public:
    SdlPlatformSharedState(SdlPlatform *owner, PlatformLogger *logger);
    ~SdlPlatformSharedState();

    void LogSdlError(const char *sdlFunctionName);

    void RegisterWindow(SdlWindow *window);
    void UnregisterWindow(SdlWindow *window);

    void RegisterGamepad(SdlGamepad *gamepad);
    void UnregisterGamepad(SdlGamepad *gamepad);

    /* We are using our own id numbering to prevent id jumping and also we can't rely on cdevice.which as SDL doesn't guarantee that it won't change.
     * In another words connection event can be received with one cdevice.which and disconnect event with completely different cdevice.which.
     * So these two functions and additionally SdlGamepad::JoystickId implements that. */
    int32_t GetGamepadIdBySdlId(uint32_t id);
    int32_t NextFreeGamepadId();

    std::vector<uint32_t> AllWindowIds() const;
    SdlWindow *Window(uint32_t id) const;
    SdlGamepad *Gamepad(uint32_t id) const;

 private:
    SdlPlatform *owner_ = nullptr;
    PlatformLogger *logger_ = nullptr;
    std::unordered_map<uint32_t, SdlWindow *> windowById_;
    std::unordered_map<uint32_t, SdlGamepad *> gamepadById_;
};
