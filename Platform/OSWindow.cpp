#include "Platform/OSWindow.h"

#include "Engine/IocContainer.h"

using EngineIoc = Engine_::IocContainer;


OSWindow::OSWindow() {
    this->mouse = EngineIoc::ResolveMouse();
}
