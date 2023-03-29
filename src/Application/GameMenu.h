#pragma once

#include <memory>

#include "Engine/EngineIocContainer.h"

#include "Io/Mouse.h"

using Io::Mouse;

class Menu {
 public:
    inline Menu() {
        this->mouse = EngineIocContainer::ResolveMouse();
    }

    void MenuLoop();

 private:
     std::shared_ptr<Mouse> mouse = nullptr;

     void EventLoop();
};
