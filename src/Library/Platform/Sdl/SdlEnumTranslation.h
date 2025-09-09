#pragma once

#include <SDL3/SDL.h>

#include "Library/Platform/Interface/PlatformEnums.h"
#include "Library/Platform/Interface/PlatformOpenGLOptions.h"

#include "Library/Logger/LogEnums.h"

PlatformKey translateSdlKey(SDL_Scancode key);
PlatformKey translateSdlGamepadButton(SDL_GamepadButton button);
PlatformKey translateSdlGamepadAxis(SDL_GamepadAxis axis);
PlatformModifiers translateSdlMods(uint16_t mods);
PlatformMouseButton translateSdlMouseButton(uint8_t mouseButton);
PlatformMouseButtons translateSdlMouseButtons(uint32_t mouseButtons);
int translatePlatformVSyncMode(PlatformVSyncMode vsyncMode);
SDL_GLProfile translatePlatformOpenGLProfile(PlatformOpenGLProfile profile);
SDL_LogPriority translatePlatformLogLevel(LogLevel logLevel);
LogLevel translateSdlLogLevel(SDL_LogPriority logLevel);
