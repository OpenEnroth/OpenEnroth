#pragma once

#include <SDL.h>
#include <SDL_joystick.h>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>

#include "Platform/Sdl/SdlGamepad.h"

class SdlWindow;
class SdlGamepad;
class PlatformLogger;
class PlatformGamepad;

class SdlPlatformSharedState {
 public:
    explicit SdlPlatformSharedState(PlatformLogger *logger);
    ~SdlPlatformSharedState();

    void logSdlError(const char *sdlFunctionName);

    void registerWindow(SdlWindow *window);
    void unregisterWindow(SdlWindow *window);
    std::vector<uint32_t> allWindowIds() const;
    SdlWindow *window(uint32_t id) const;

    void initializeGamepads();
    SdlGamepad *initializeGamepad(int gamepadId); // Note: it's gamepad id, not joystick id.
    void deinitializeGamepad(SDL_JoystickID id); // And here it's joystick id, which is a different from a gamepad id!
    std::vector<PlatformGamepad *> allGamepads() const;
    SdlGamepad *gamepad(SDL_JoystickID id) const;

 private:
    PlatformLogger *_logger = nullptr;
    std::unordered_map<uint32_t, SdlWindow *> _windowById;
    std::unordered_map<SDL_JoystickID, std::unique_ptr<SdlGamepad>> _gamepadById;
};
