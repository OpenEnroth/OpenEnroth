#include "Platform/Sdl2Platform.h"

#include <SDL2/SDL.h>

void OS_MsgBox(const char *msg, const char *title) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, msg, nullptr);
}
