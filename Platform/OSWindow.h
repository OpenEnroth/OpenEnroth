#pragma once
#include "Engine/Point.h"

extern class OSWindow *window;

class Mouse;
class OSWindow {
 public:
    virtual OSWindow();
    static OSWindow *Create(const wchar_t *title, int window_width,
                            int window_height);

    virtual void SetFullscreenMode();
    virtual void SetWindowedMode(int new_window_width, int new_window_height);
    virtual void SetCursor(const char *cursor_name);

    virtual void *GetApiHandle() const;
    virtual int GetX() const;
    virtual int GetY() const;
    virtual unsigned int GetWidth() const;
    virtual unsigned int GetHeight() const;

    virtual Point TransformCursorPos(Point &pt) const;  // screen to client

    virtual bool OnOSMenu(int item_id);

    virtual void Show();
    virtual bool Focused();
    virtual bool OnMouseLeftClick(int x, int y);
    virtual bool OnMouseRightClick(int x, int y);
    virtual bool Activate();
};
