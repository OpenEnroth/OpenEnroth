#pragma once

#include <SDL.h>

#include <utility>

#include "Platform/PlatformKey.h"
#include "Platform/PlatformModifiers.h"
#include "Platform/PlatformMouseButton.h"
#include "Platform/PlatformOpenGLOptions.h"
#include "Platform/PlatformLogger.h"

PlatformKey TranslateSdlKey(SDL_Scancode key);
PlatformKey TranslateSdlGamepadButton(SDL_GameControllerButton button);
std::pair<PlatformKey, PlatformKeyType> TranslateSdlGamepadAxis(SDL_GameControllerAxis axis, float value);
PlatformModifiers TranslateSdlMods(uint16_t mods);
PlatformMouseButton TranslateSdlMouseButton(uint8_t mouseButton);
PlatformMouseButtons TranslateSdlMouseButtons(uint32_t mouseButtons);
int TranslatePlatformVSyncMode(PlatformVSyncMode vsyncMode);
SDL_GLprofile TranslatePlatformOpenGLProfile(PlatformOpenGLProfile profile);
SDL_LogPriority TranslatePlatformLogLevel(PlatformLogLevel logLevel);
PlatformLogLevel TranslateSdlLogLevel(SDL_LogPriority logLevel);
