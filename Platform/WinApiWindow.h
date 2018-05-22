#pragma once

#include <Windows.h>

#include "Platform/OSWindow.h"

void *WinApiMsgRouter(HWND hwnd, int msg, int wparam, void *lparam);

class WinApiWindow : public OSWindow {
 public:
    inline WinApiWindow(HWND hwnd)
        : OSWindow() {
        this->hwnd = hwnd;
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
     bool WinApiMessageProc(int msg, int wparam, void *lparam, void **result);
     void *CreateDebugMenuPanel();


     HWND hwnd = nullptr;

     friend void *WinApiMsgRouter(HWND hwnd, int msg, int wparam, void *lparam);
};
