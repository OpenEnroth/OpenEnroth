#include "Platform/OSWindow.h"

#include "Engine/IocContainer.h"

#include "Application/IocContainer.h"


std::shared_ptr<OSWindow> window = nullptr;

using EngineIoc = Engine_::IocContainer;
using ApplicationIoc = Application::IocContainer;


OSWindow::OSWindow() {
    this->gameCallback = ApplicationIoc::ResolveGameWindowHandler();
    this->log = EngineIoc::ResolveLogger();
}
