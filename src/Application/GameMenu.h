#pragma once

#include <memory>

namespace Io {
class Mouse;
} // namespace Io

class Menu {
 public:
    Menu();
    ~Menu();

    void MenuLoop();

 private:
     std::shared_ptr<Io::Mouse> mouse = nullptr;

     void EventLoop();
};
