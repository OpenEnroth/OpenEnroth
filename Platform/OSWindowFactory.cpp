#include "Platform/OSWindowFactory.h"

#ifdef _WINDOWS
#include <Windows.h>
#else
#include <cstring>
#endif
#include <stdio.h>

#include "Platform/Api.h"
#include "Platform/Sdl2Window.h"

OSWindow *OSWindowFactory::Create(const char *title, int window_width, int window_height) {
    return CreateSdl2(title, window_width, window_height);
}

OSWindow *OSWindowFactory::CreateSdl2(const char *title, int window_width, int window_height) {
    auto sdlWindow = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        window_width, window_height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    if (sdlWindow) {
        auto sdlWindowSurface = SDL_GetWindowSurface(sdlWindow);
        if (sdlWindowSurface) {
            return new Sdl2Window(sdlWindow, sdlWindowSurface);
        } else {
            SDL_DestroyWindow(sdlWindow);
        }
    }

    return nullptr;
}

int main(int argc, char *argv[]) {
    extern int MM_Main(const char *);
    return MM_Main(argv[0]);
}
