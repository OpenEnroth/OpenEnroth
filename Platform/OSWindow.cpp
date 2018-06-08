#include "Platform/OSWindow.h"

#include "Engine/IocContainer.h"

#include "src/Application/IocContainer.h"


using EngineIoc = Engine_::IocContainer;
using ApplicationIoc = Application::IocContainer;


OSWindow::OSWindow() {
    this->gameCallback = ApplicationIoc::ResolveGameWindowHandler();
    this->log = EngineIoc::ResolveLogger();
}
