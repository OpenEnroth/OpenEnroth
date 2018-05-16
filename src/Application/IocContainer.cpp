#include "src/Application/IocContainer.h"

#include "src/Application/GameMenu.h"

using Application::IocContainer;
using Application::Menu;

Menu *IocContainer::menu = nullptr;
Menu *IocContainer::ResolveGameMenu() {
    if (!menu) {
        menu = new Menu();
    }
    return menu;
}
