#pragma once

#include <SDL.h>

#include "Platform/PlatformEnums.h"
#include "Platform/PlatformOpenGLOptions.h"

PlatformKey translateSdlKey(SDL_Scancode key);
PlatformKey translateSdlGamepadButton(SDL_GameControllerButton button);
PlatformKey translateSdlGamepadAxis(SDL_GameControllerAxis axis);
PlatformModifiers translateSdlMods(uint16_t mods);
PlatformMouseButton translateSdlMouseButton(uint8_t mouseButton);
PlatformMouseButtons translateSdlMouseButtons(uint32_t mouseButtons);
int translatePlatformVSyncMode(PlatformVSyncMode vsyncMode);
SDL_GLprofile translatePlatformOpenGLProfile(PlatformOpenGLProfile profile);
SDL_LogPriority translatePlatformLogLevel(PlatformLogLevel logLevel);
PlatformLogLevel translateSdlLogLevel(SDL_LogPriority logLevel);
