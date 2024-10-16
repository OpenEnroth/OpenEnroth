#pragma once

#include <SDL.h>

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>

#include "Library/Logger/LogCategory.h"

class SdlWindow;
class SdlEventLoop;
class SdlPlatform;
class SdlGamepad;
class PlatformEvent;
class PlatformEventHandler;
class PlatformGamepad;
class Logger;

class SdlPlatformSharedState {
 public:
    SdlPlatformSharedState();
    ~SdlPlatformSharedState();

    void logSdlError(const char *sdlFunctionName);
    static const LogCategory &logCategory();

    void registerWindow(SdlWindow *window);
    void unregisterWindow(SdlWindow *window);
    std::vector<uint32_t> allWindowIds() const;
    SdlWindow *window(uint32_t id) const;

    void registerEventLoop(SdlEventLoop *eventLoop);
    void unregisterEventLoop(SdlEventLoop *eventLoop);

    void initializeGamepads();
    SdlGamepad *initializeGamepad(int gamepadId); // Note: it's gamepad id, not joystick id.
    void deinitializeGamepad(SDL_JoystickID id); // And here it's joystick id, which is a different from a gamepad id!
    std::vector<PlatformGamepad *> allGamepads() const;
    SdlGamepad *gamepad(SDL_JoystickID id) const;

 private:
    std::unordered_map<uint32_t, SdlWindow *> _windowById;
    std::unordered_map<SDL_JoystickID, std::unique_ptr<SdlGamepad>> _gamepadById;
    size_t _eventLoopCount = 0;
};
