#pragma once
#include "Engine/Point.h"


class Mouse;
class OSWindow {
 public:
    OSWindow();

    virtual void SetFullscreenMode() = 0;
    virtual void SetWindowedMode(int new_window_width, int new_window_height) = 0;
    virtual void SetCursor(const char *cursor_name) = 0;

    virtual int GetX() const = 0;
    virtual int GetY() const = 0;
    virtual unsigned int GetWidth() const = 0;
    virtual unsigned int GetHeight() const = 0;

    virtual Point TransformCursorPos(Point &pt) const = 0;  // screen to client

    virtual bool OnOSMenu(int item_id) = 0;

    virtual void Show() = 0;
    virtual bool Focused() = 0;
    virtual bool OnMouseLeftClick(int x, int y) = 0;
    virtual bool OnMouseRightClick(int x, int y) = 0;
    virtual bool Activate() = 0;

    virtual void *GetWinApiHandle() = 0;

 protected:
    Mouse *mouse = nullptr;
};

extern OSWindow *window;
