#pragma once

class OSWindow;
class OSWindowFactory {
 public:
    OSWindow *Create(const char *title, int window_width, int window_height);

 private:
    OSWindow *CreateSdl2(const char *title, int window_width, int window_height);
};
