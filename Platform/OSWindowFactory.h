#pragma once

#include <memory>

class OSWindow;
class OSWindowFactory {
 public:
    std::shared_ptr<OSWindow> Create(const char* title, int x, int y, int width, int height, int display, int fullscreen, int borderless);
};
