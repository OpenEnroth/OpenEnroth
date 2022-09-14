#include <SDL.h>

#include "Platform/Sdl2MouseController.h"

int Sdl2MouseController::GetCursorX() const {
    int x, y;
    SDL_GetMouseState(&x, &y);

    return x;
}

int Sdl2MouseController::GetCursorY() const {
    int x, y;
    SDL_GetMouseState(&x, &y);

    return y;
}
