#include "Platform/OSWindowFactory.h"

#include "Platform/Api.h"
#include "Platform/Sdl2Window.h"

std::shared_ptr<OSWindow> CreateSdl2Window(const char *title, int x, int y, int width, int height, int display, int fullscreen, int borderless) {
    std::vector<SDL_Rect> displayBounds;
    Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;

    int displays = SDL_GetNumVideoDisplays();
    if (displays < display)
        display = 0;

    for (int i = 0; i < displays; i++) {
        displayBounds.push_back(SDL_Rect());
        SDL_GetDisplayBounds(i, &displayBounds.back());
    }

    if (fullscreen) {
        if (borderless)
            flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        else
            flags |= SDL_WINDOW_FULLSCREEN;

        x = SDL_WINDOWPOS_CENTERED_DISPLAY(display);
        y = SDL_WINDOWPOS_CENTERED_DISPLAY(display);
    } else {
        if (borderless)
            flags |= SDL_WINDOW_BORDERLESS;

        if (x >= 0 && x < displayBounds[display].w)
            x += displayBounds[display].x;
        else
            x = SDL_WINDOWPOS_CENTERED_DISPLAY(display);

        if (y >= 0 && y < displayBounds[display].h)
            y += displayBounds[display].y;
        else
            y = SDL_WINDOWPOS_CENTERED_DISPLAY(display);
    }

    displayBounds.clear();
    auto sdlWindow = SDL_CreateWindow(
        title,
        x, y,
        width, height,
        flags
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

std::shared_ptr<OSWindow> OSWindowFactory::Create(const char* title, int display, int x, int y, int window_width, int window_height, int fullscreen, int borderless) {
    return CreateSdl2Window(title, display, x, y, window_width, window_height, fullscreen, borderless);
}
