#pragma once
#include "Engine/Point.h"
#include "Platform/OSWindow.h"

extern class OSWindow *window;

class Mouse;
class WinOSWindow : OSWindow {
 public:
    WinOSWindow();
    static OSWindow *Create(const wchar_t *title, int window_width,
                            int window_height);

    void SetFullscreenMode();
    void SetWindowedMode(int new_window_width, int new_window_height);
    void SetCursor(const char *cursor_name);

    void *GetApiHandle() const;
    int GetX() const;
    int GetY() const;
    unsigned int GetWidth() const;
    unsigned int GetHeight() const;

    Point TransformCursorPos(Point &pt) const;  // screen to client

    bool OnOSMenu(int item_id);

    virtual void Show();
    virtual bool Focused();
    virtual bool OnMouseLeftClick(int x, int y);
    virtual bool OnMouseRightClick(int x, int y);
    virtual bool Activate();

 protected:
    bool Initialize(const wchar_t *title, int window_width, int window_height);
    bool WinApiMessageProc(int msg, int wparam, void *lparam, void **result);

    void *api_handle;

 private:
    static void *WinApiMsgRouter(void *hwnd, int msg, int wparam, void *lparam);

    void *CreateDebugMenuPanel();

    Mouse *mouse = nullptr;
};
