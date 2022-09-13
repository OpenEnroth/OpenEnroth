#pragma once

#include <memory>

#include "Application/GameConfig.h"

class OSWindow;
class OSWindowFactory {
 public:
    std::shared_ptr<OSWindow> Create(std::shared_ptr<Application::GameConfig> config);
};
