#include "Platform/OSWindowFactory.h"

#include "Platform/Api.h"
#include "Platform/Sdl2Window.h"

std::shared_ptr<OSWindow> OSWindowFactory::Create() {
    auto window = std::make_shared<Sdl2Window>();
    auto sdlWindow = window->CreateSDLWindow();

    if (sdlWindow) {
        window->HandleAllEvents();

        return window;
    }

    return nullptr;
}
