#pragma once
#include <memory>

#include "Engine/Point.h"
#include "Engine/Log.h"

#include "src/Application/GameWindowHandler.h"

#include "Io/IKeyboardController.h"
#include "Io/IMouseController.h"


using Application::GameWindowHandler;
using Io::IKeyboardController;
using Io::IMouseController;

#define MessageLoopWithWait()           \
    window->HandleAllEvents();          \
    if (dword_6BE364_game_settings_1 & GAME_SETTINGS_APP_INACTIVE) {    \
        window->WaitSingleEvent();      \
        continue;                       \
    }                                   \


class OSWindow {
 public:
    OSWindow();
    virtual ~OSWindow() {}

    virtual void SetFullscreenMode() = 0;
    virtual void SetWindowedMode(int new_window_width, int new_window_height) = 0;
    virtual void SetCursor(bool on) = 0;

    virtual int GetX() const = 0;
    virtual int GetY() const = 0;
    virtual unsigned int GetWidth() const = 0;
    virtual unsigned int GetHeight() const = 0;
    virtual void SetWindowArea(int width, int height) = 0;

    virtual bool OnOSMenu(int item_id) = 0;

    virtual void Show() = 0;
    virtual bool Focused() = 0;
    virtual void Activate() = 0;

    virtual void WaitSingleEvent() = 0;
    virtual void HandleSingleEvent() = 0;
    virtual void HandleAllEvents() = 0;

    virtual void *GetWinApiHandle() = 0;

    virtual std::shared_ptr<IKeyboardController> GetKeyboardController() = 0;
    virtual std::shared_ptr<IMouseController> GetMouseController() = 0;

    // window-renderer integration, probably should be a separate class
    virtual void OpenGlCreate() = 0;
    virtual void OpenGlSwapBuffers() = 0;

 protected:
    GameWindowHandler *gameCallback = nullptr;
    Log *log = nullptr;
};

extern std::shared_ptr<OSWindow> window;
