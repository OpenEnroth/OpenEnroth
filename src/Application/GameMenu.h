#pragma once

#include "Engine/IocContainer.h"

#include "Io/Mouse.h"

using EngineIoc = Engine_::IocContainer;
using Io::Mouse;

namespace Application {

class Menu {
 public:
    inline Menu() {
        this->mouse = EngineIoc::ResolveMouse();
    }

    void MenuLoop();

 private:
     std::shared_ptr<Mouse> mouse = nullptr;

     void EventLoop();
};

}  // namespace Application
