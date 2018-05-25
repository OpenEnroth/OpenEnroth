#include "Platform/OSWindow.h"

#include "src/Application/IocContainer.h"


using ApplicationIoc = Application::IocContainer;


OSWindow::OSWindow() {
    this->gameCallback = ApplicationIoc::ResolveGameWindowHandler();
}
