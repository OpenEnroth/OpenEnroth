#include "Platform/OSWindowFactory.h"

#include "Platform/Api.h"
#include "Platform/Sdl2Window.h"

std::shared_ptr<OSWindow> CreateSdl2Window(const char *title, int window_width, int window_height) {
    auto sdlWindow = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        window_width, window_height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
    );

    if (sdlWindow) {
        auto sdlWindowSurface = SDL_GetWindowSurface(sdlWindow);
        if (sdlWindowSurface) {
            auto window = std::make_shared<Sdl2Window>(sdlWindow, sdlWindowSurface);
            window->HandleAllEvents();

            return window;
        } else {
            SDL_DestroyWindow(sdlWindow);
        }
    }

    return nullptr;
}

std::shared_ptr<OSWindow> OSWindowFactory::Create(const char* title, int window_width, int window_height) {
    return CreateSdl2Window(title, window_width, window_height);
}