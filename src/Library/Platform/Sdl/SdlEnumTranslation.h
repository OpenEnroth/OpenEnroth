#pragma once

#include <SDL.h>

#include <utility>

#include "Library/Platform/Interface/PlatformEnums.h"
#include "Library/Platform/Interface/PlatformOpenGLOptions.h"

#include "Library/Logger/LogEnums.h"

PlatformKey translateSdlKey(SDL_Scancode key);
PlatformKey translateSdlGamepadButton(SDL_GameControllerButton button);
PlatformKey translateSdlGamepadAxis(SDL_GameControllerAxis axis);
PlatformModifiers translateSdlMods(uint16_t mods);
PlatformMouseButton translateSdlMouseButton(uint8_t mouseButton);
PlatformMouseButtons translateSdlMouseButtons(uint32_t mouseButtons);
int translatePlatformVSyncMode(PlatformVSyncMode vsyncMode);
SDL_GLprofile translatePlatformOpenGLProfile(PlatformOpenGLProfile profile);
SDL_LogPriority translatePlatformLogLevel(LogLevel logLevel);
LogLevel translateSdlLogLevel(SDL_LogPriority logLevel);
