#include "Platform/OSWindowFactory.h"

#include "Platform/Api.h"
#include "Platform/Sdl2Window.h"

std::shared_ptr<OSWindow> OSWindowFactory::Create(std::shared_ptr<Application::GameConfig> config) {
    auto window = std::make_shared<Sdl2Window>(config);
    auto sdlWindow = window->CreateSDLWindow();

    if (sdlWindow) {
        window->HandleAllEvents();

        return window;
    }

    return nullptr;
}
