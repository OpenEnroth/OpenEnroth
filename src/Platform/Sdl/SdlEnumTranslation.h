#pragma once

#include <SDL.h>

#include "Platform/PlatformKey.h"
#include "Platform/PlatformModifiers.h"
#include "Platform/PlatformMouseButton.h"
#include "Platform/PlatformOpenGLOptions.h"

PlatformKey TranslateSdlKey(SDL_Scancode key);
PlatformModifiers TranslateSdlMods(uint16_t mods);
PlatformMouseButton TranslateSdlMouseButton(uint8_t mouseButton);
PlatformMouseButtons TranslateSdlMouseButtons(uint32_t mouseButtons);
int TranslatePlatformVSyncMode(PlatformVSyncMode vsyncMode);
SDL_GLprofile TranslatePlatformOpenGLProfile(PlatformOpenGLProfile profile);
