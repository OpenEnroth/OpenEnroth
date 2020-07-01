#pragma once
#include <memory>

#include <SDL.h>

#include "Platform/OSWindow.h"
#include "Platform/Sdl2KeyboardController.h"
#include "Platform/Sdl2MouseController.h"

class Sdl2Window : public OSWindow {
 public:
    inline Sdl2Window(SDL_Window *sdlWindow, SDL_Surface *sdlWindowSurface)
        : OSWindow() {
        this->sdlWindow = sdlWindow;
        this->sdlWindowSurface = sdlWindowSurface;
    }

    void SetFullscreenMode() override;
    void SetWindowedMode(int new_window_width, int new_window_height) override;
    void SetCursor(bool on) override;

    int GetX() const override;
    int GetY() const override;
    unsigned int GetWidth() const override;
    unsigned int GetHeight() const override;
    void SetWindowArea(int width, int height) override;

    bool OnOSMenu(int item_id) override;

    void Show() override;
    bool Focused() override;
    void Activate() override;

    void WaitSingleEvent() override;
    void HandleSingleEvent() override;
    void HandleAllEvents() override;

    std::shared_ptr<IKeyboardController> GetKeyboardController() override {
        return std::make_shared<Sdl2KeyboardController>();
    }

    std::shared_ptr<IMouseController> GetMouseController() override {
        return std::make_shared<Sdl2MouseController>();
    }

    // Sdl2Window-specific interface follows

    virtual void* GetApiHandle() override {
        return sdlWindow;
    }

    void* GetWinApiHandle() override;

    // window<->renderer integration, probably should be a separate class
    void OpenGlCreate() override;
    void OpenGlSwapBuffers() override;

 private:
    SDL_Window *sdlWindow = nullptr;
    SDL_Surface *sdlWindowSurface = nullptr;
    SDL_GLContext sdlOpenGlContext = nullptr;

    void MessageProc(const SDL_Event &e);
    bool TryMapScanCode(SDL_Scancode code, GameKey* outKey) const;
    bool TryMapKeyCode(SDL_Keycode key, bool uppercase, int *outKey) const;
};
