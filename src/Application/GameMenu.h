#pragma once

#include "Engine/IocContainer.h"

using EngineIoc = Engine_::IocContainer;

namespace Application {

class Menu {
 public:
    inline Menu() {
        this->mouse = EngineIoc::ResolveMouse();
    }

    void MenuLoop();

 private:
     Mouse *mouse = nullptr;

     void EventLoop();
};

}  // namespace Application
