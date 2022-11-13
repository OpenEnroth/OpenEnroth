#pragma once
#include <memory>

#include <SDL.h>

#include "Platform/OSWindow.h"
#include "Platform/Sdl2KeyboardController.h"
#include "Platform/Sdl2MouseController.h"

class Sdl2Window : public OSWindow {
 public:
    inline Sdl2Window(std::shared_ptr<Application::GameConfig> config): OSWindow() {
        this->config = config;
    }

    void SetFullscreenMode() override;
    void SetWindowedMode(int new_window_width, int new_window_height) override;
    void SetCursor(bool on) override;

    int GetX() const override;
    int GetY() const override;
    unsigned int GetWidth() const override;
    unsigned int GetHeight() const override;
    void SetWindowArea(int width, int height) override;
    void SaveWindowPosition();

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
    void* GetWinApiHandle() override;
    SDL_Window* getSDLWindow();
    SDL_GLContext* getSDLOpenGlContext();
    SDL_Window* CreateSDLWindow();
    void Release() override;

    // window<->renderer integration, probably should be a separate class
    void OpenGlCreate() override;
    void OpenGlSwapBuffers() override;
    std::shared_ptr<Application::GameConfig> config = nullptr;

 private:
    struct Sdl2WinParams {
        Uint32 flags;
        int x;
        int y;
        int display;
    };
    SDL_Window *sdlWindow = nullptr;
    SDL_GLContext sdlOpenGlContext = nullptr;

    Sdl2WinParams CalculateWindowParameters();
    bool NuklearEventHandler(const SDL_Event &e);
    void MessageProc(const SDL_Event &e);
    bool TryMapScanCode(SDL_Scancode code, PlatformKey* outKey) const;
    bool TryMapKeyCode(SDL_Keycode key, bool uppercase, int *outKey) const;
};
