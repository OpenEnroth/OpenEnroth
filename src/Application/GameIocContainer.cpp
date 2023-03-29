#include "GameIocContainer.h"

#include "GameMenu.h"
#include "GameWindowHandler.h"

Menu *GameIocContainer::menu = nullptr;
Menu *GameIocContainer::ResolveGameMenu() {
    if (!menu) {
        menu = new Menu();
    }
    return menu;
}

GameWindowHandler *GameIocContainer::gameWindowHandler = nullptr;
GameWindowHandler *GameIocContainer::ResolveGameWindowHandler() {
    if (!gameWindowHandler) {
        gameWindowHandler = new GameWindowHandler();
    }
    return gameWindowHandler;
}
