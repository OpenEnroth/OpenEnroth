#pragma once
#include "Engine/Point.h"

#include "GUI/NewUI/Core/UIControl.h"

extern class OSWindow *window;

class OSWindow : public UIControl
{
    public:
        static OSWindow *Create(const wchar_t *title, int window_width, int window_height);

        void SetFullscreenMode();
        void SetWindowedMode(int new_window_width, int new_window_height);
        void SetCursor(const char *cursor_name);

        void         *GetApiHandle() const;
        int           GetX() const;
        int           GetY() const;
        unsigned int  GetWidth() const;
        unsigned int  GetHeight() const;

        Point TransformCursorPos(Point &pt) const; // screen to client


        bool OnOSMenu(int item_id);

        // UIControl
        virtual void Show() override;

        // UIControl
        virtual bool Focused() override;

        // UIControl
        //virtual bool OnKey(int key) override;
        // UIControl
        virtual bool OnMouseLeftClick(int x, int y) override;
        // UIControl
        virtual bool OnMouseRightClick(int x, int y) override;
        // UIControl
        //virtual bool OnMouseEnter() override;
        // UIControl
        //virtual bool OnMouseLeave() override;

        virtual bool Activate();

    protected:
        bool Initialize(const wchar_t *title, int window_width, int window_height);
        bool WinApiMessageProc(int msg, int wparam, void *lparam, void **result);

        void *api_handle;

    private:
        static void *__stdcall WinApiMsgRouter(void *hwnd, int msg, int wparam, void *lparam);

        void *CreateDebugMenuPanel();
};