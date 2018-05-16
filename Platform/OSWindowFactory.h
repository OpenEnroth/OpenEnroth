#pragma once

class OSWindow;
class OSWindowFactory {
 public:
    OSWindow *Create(const wchar_t *title, int window_width, int window_height);

 private:
    OSWindow *CreateSdl2(const wchar_t *title, int window_width, int window_height);
    OSWindow *CreateWinApi(const wchar_t *title, int window_width, int window_height);
};