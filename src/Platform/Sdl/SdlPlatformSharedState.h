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
    explicit SdlPlatformSharedState(PlatformLogger *logger);
    ~SdlPlatformSharedState();

    void logSdlError(const char *sdlFunctionName);

    void registerWindow(SdlWindow *window);
    void unregisterWindow(SdlWindow *window);

    void registerGamepad(SdlGamepad *gamepad);
    void unregisterGamepad(SdlGamepad *gamepad);

    /* We are using our own id numbering to prevent id jumping and also we can't rely on cdevice.which as SDL doesn't guarantee that it won't change.
     * In another words connection event can be received with one cdevice.which and disconnect event with completely different cdevice.which.
     * So these two functions and additionally SdlGamepad::JoystickId implements that. */
    int32_t getGamepadIdBySdlId(uint32_t id);
    int32_t nextFreeGamepadId();

    std::vector<uint32_t> allWindowIds() const;
    SdlWindow *window(uint32_t id) const;
    SdlGamepad *gamepad(uint32_t id) const;

 private:
    PlatformLogger *_logger = nullptr;
    std::unordered_map<uint32_t, SdlWindow *> _windowById;
    std::unordered_map<uint32_t, SdlGamepad *> _gamepadById;
};
