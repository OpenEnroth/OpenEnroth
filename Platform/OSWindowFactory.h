#pragma once

#include <memory>

class OSWindow;
class OSWindowFactory {
 public:
    std::shared_ptr<OSWindow> Create(const char *title, int window_width, int window_height);
};
