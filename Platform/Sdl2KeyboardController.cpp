#include <SDL.h>

#include <algorithm>
#include <map>

#include "Platform/Sdl2KeyboardController.h"

using Io::GameKey;

extern int SDL_GetAsyncKeyState(GameKey key, bool consume);

bool Sdl2KeyboardController::IsKeyPressed(GameKey key) const {
    return SDL_GetAsyncKeyState(key, true) == 1;
}

bool Sdl2KeyboardController::IsKeyHeld(GameKey key) const {
    int status = SDL_GetAsyncKeyState(key, false);
    return status == 1 || status == 2;
}
