#include <SDL.h>

#include <algorithm>
#include <map>

#include "Platform/Sdl2KeyboardController.h"

extern int SDL_GetAsyncKeyState(PlatformKey key, bool consume);

bool Sdl2KeyboardController::IsKeyPressed(PlatformKey key) const {
    return SDL_GetAsyncKeyState(key, true) == 1;
}

bool Sdl2KeyboardController::IsKeyHeld(PlatformKey key) const {
    int status = SDL_GetAsyncKeyState(key, false);
    return status == 1 || status == 2;
}
