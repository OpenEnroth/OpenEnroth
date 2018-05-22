#pragma once

#include "lib/SDL2/include/SDL.h"

#include "Platform/OSWindow.h"

class Sdl2Window : public OSWindow {
 public:
    inline Sdl2Window(SDL_Window *sdlWindow, SDL_Surface *sdlWindowSurface)
        : OSWindow() {
        this->sdlWindow = sdlWindow;
        this->sdlWindowSurface = sdlWindowSurface;
    }

    void SetFullscreenMode() override;
    void SetWindowedMode(int new_window_width, int new_window_height) override;
    void SetCursor(const char *cursor_name) override;

    int GetX() const override;
    int GetY() const override;
    unsigned int GetWidth() const override;
    unsigned int GetHeight() const override;

    Point TransformCursorPos(Point &pt) const override;  // screen to client

    bool OnOSMenu(int item_id) override;

    void Show() override;
    bool Focused() override;
    void Activate() override;

    void PeekSingleMessage() override;
    void PeekMessageLoop() override;

    void *GetWinApiHandle() override;

 private:
    SDL_Window *sdlWindow;
    SDL_Surface *sdlWindowSurface;


    void MessageProc(const SDL_Event &e);
    bool SdlkIsChar(SDL_Keycode key) const;
};
