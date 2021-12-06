#pragma once

#include <memory>

class OSWindow;
class OSWindowFactory {
 public:
    std::shared_ptr<OSWindow> Create();
};
