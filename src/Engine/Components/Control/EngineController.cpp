#include "EngineController.h"

#include <cassert>
#include <utility>
#include <thread>
#include <string>
#include <memory>

#include "Arcomage/Arcomage.h"

#include "GUI/GUIProgressBar.h"
#include "GUI/GUIWindow.h"
#include "GUI/GUIButton.h"

#include "Engine/Engine.h"
#include "Engine/SaveLoad.h"
#include "Engine/EngineFileSystem.h"
#include "Engine/EngineGlobals.h"
#include "Engine/mm7_data.h"

#include "Library/FileSystem/Memory/MemoryFileSystem.h"
#include "Library/Platform/Application/PlatformApplication.h"
#include "Library/Platform/Interface/PlatformEvents.h"

#include "Utility/Exception.h"
#include "Utility/ScopedRollback.h"

EngineController::EngineController(EngineControlStateHandle state): _state(std::move(state)) {}

EngineController::~EngineController() = default;

void EngineController::tick(int count) {
    for (int i = 0; i < count; i++) {
        _state.yieldExecution();

        // We should check `terminating` after a call to `yieldExecution` because it cannot be set before the call -
        // the only place it's set is the main thread, and main thread wasn't running before the call.
        if (_state->terminating)
            throw EngineControlState::TerminationException();
    }
}

void EngineController::postEvent(std::unique_ptr<PlatformEvent> event) {
    _state->postedEvents.push(std::move(event));
}

void EngineController::pressKey(PlatformKey key) {
    std::unique_ptr<PlatformKeyEvent> event = std::make_unique<PlatformKeyEvent>();
    event->type = EVENT_KEY_PRESS;
    event->window = ::application->window();
    event->key = key;
    event->mods = 0;
    event->isAutoRepeat = false;
    postEvent(std::move(event));
}

void EngineController::pressAutoRepeatedKey(PlatformKey key) {
    std::unique_ptr<PlatformKeyEvent> event = std::make_unique<PlatformKeyEvent>();
    event->type = EVENT_KEY_PRESS;
    event->window = ::application->window();
    event->key = key;
    event->mods = 0;
    event->isAutoRepeat = true;
    postEvent(std::move(event));
}

void EngineController::releaseKey(PlatformKey key) {
    std::unique_ptr<PlatformKeyEvent> event = std::make_unique<PlatformKeyEvent>();
    event->type = EVENT_KEY_RELEASE;
    event->window = ::application->window();
    event->key = key;
    event->mods = 0;
    event->isAutoRepeat = false;
    postEvent(std::move(event));
}

void EngineController::pressButton(PlatformMouseButton button, int x, int y) {
    std::unique_ptr<PlatformMouseEvent> event = std::make_unique<PlatformMouseEvent>();
    event->type = EVENT_MOUSE_BUTTON_PRESS;
    event->window = ::application->window();
    event->button = BUTTON_LEFT;
    event->pos = Pointi(x, y);
    event->isDoubleClick = false;
    postEvent(std::move(event));
}

void EngineController::releaseButton(PlatformMouseButton button, int x, int y) {
    std::unique_ptr<PlatformMouseEvent> event = std::make_unique<PlatformMouseEvent>();
    event->type = EVENT_MOUSE_BUTTON_RELEASE;
    event->window = ::application->window();
    event->button = BUTTON_LEFT;
    event->buttons = BUTTON_LEFT;
    event->pos = Pointi(x, y);
    event->isDoubleClick = false;
    postEvent(std::move(event));
}

void EngineController::moveMouse(int x, int y) {
    std::unique_ptr<PlatformMouseEvent> event = std::make_unique<PlatformMouseEvent>();
    event->type = EVENT_MOUSE_MOVE;
    event->window = ::application->window();
    event->button = BUTTON_NONE;
    event->buttons = BUTTON_NONE;
    event->pos = Pointi(x, y);
    event->isDoubleClick = false;
    postEvent(std::move(event));
}

void EngineController::pressAndReleaseKey(PlatformKey key) {
    pressKey(key);
    releaseKey(key);
}

void EngineController::pressAndReleaseButton(PlatformMouseButton button, int x, int y) {
    pressButton(button, x, y);
    releaseButton(button, x, y);
}

void EngineController::pressGuiButton(std::string_view buttonId) {
    GUIButton *button = existingButton(buttonId);

    pressAndReleaseButton(BUTTON_LEFT, button->uX + button->uWidth / 2, button->uY + button->uHeight / 2);
}

void EngineController::goToMainMenu() {
    auto maybeThrow = [counter = 0] () mutable {
        if (++counter >= 128)
            throw Exception("Couldn't return to main menu");
    };

    engine->disableOverlays();

    // Skip movies.
    while (current_screen_type == SCREEN_VIDEO) {
        maybeThrow();
        pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
        tick(1);
    }

    // Can't always leave key settings menu by pressing ESC, so need custom handling.
    if (current_screen_type == SCREEN_KEYBOARD_OPTIONS) {
        pressGuiButton("KeyBinding_Default");
        tick(1);
    }

    // Leave to game screen if we're in the game, or to main menu if we're in menus.
    while (current_screen_type != SCREEN_GAME && GetCurrentMenuID() != MENU_MAIN) {
        maybeThrow();
        pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
        tick(2); // Somehow tick(1) is not enough when we're trying to leave the game loading menu.
    }

    // If game is starting up - wait for main menu to appear.
    while (GetCurrentMenuID() == MENU_MAIN && lWindowList.empty()) {
        maybeThrow();
        tick(1);
    }

    if (GetCurrentMenuID() == MENU_MAIN)
        return;
    assert(GetCurrentMenuID() == MENU_NONE);

    // Go to in-game menu.
    while (current_screen_type != SCREEN_MENU) {
        maybeThrow();
        pressAndReleaseKey(PlatformKey::KEY_ESCAPE);
        tick(1);
    }

    // Leave to main menu from there.
    pressGuiButton("GameMenu_Quit");
    tick(1);
    pressGuiButton("GameMenu_Quit");
    while (GetCurrentMenuID() != MENU_MAIN) {
        maybeThrow();
        tick(1);
    }
}

void EngineController::startNewGame() {
    goToMainMenu();
    pressGuiButton("MainMenu_NewGame");
    tick(2);
    pressGuiButton("PartyCreation_OK");
    skipLoadingScreen();
    tick(2);
}

void EngineController::skipLoadingScreen() {
    int steps = 0;
    while (!pGameLoadingUI_ProgressBar->IsActive()) {
        tick(1);
        steps++;
        if (steps >= 128)
            throw Exception("Can't skip a non-existent loading screen");
    }
    while (pGameLoadingUI_ProgressBar->IsActive())
        tick(1);
    while (dword_6BE364_game_settings_1 & GAME_SETTINGS_0080_SKIP_USER_INPUT_THIS_FRAME)
        tick(1);
}

Blob EngineController::saveGame() {
    // AutoSave makes a screenshot and needs the opengl context that's bound in game thread, so we cannot call it from
    // the control thread. One option is to unbind every time we switch to control thread, but this is slow, and not
    // needed 99% of the time. So we just call back into the game thread.
    Blob result;
    runGameRoutine([&] { result = CreateSaveData(false, "").second; });
    return result;
}

void EngineController::loadGame(const Blob &savedGame) {
    MemoryFileSystem ramFs("ramfs");
    ramFs.write("saves/!!!save.mm7", savedGame);

    ScopedRollback<FileSystem *> rollback(&ufs, &ramFs);

    goToMainMenu();
    pressGuiButton("MainMenu_LoadGame");
    tick(3);
    pSavegameList->saveListPosition = 0; // Make sure we start at the top of the list.
    tick(1);

    assert(pSavegameList->pSavegameUsedSlots[0]);
    assert(pSavegameList->pFileList[0] == "!!!save.mm7");

    pressGuiButton("LoadMenu_Slot0");
    tick(2);
    pressGuiButton("LoadMenu_Load");
    skipLoadingScreen();
}

void EngineController::runGameRoutine(GameRoutine routine) {
    _state->gameRoutine = std::move(routine);
    _state.yieldExecution();
    assert(!_state->gameRoutine); // Must have finished.
    if (_state->terminating)
        throw EngineControlState::TerminationException();
}

void EngineController::resizeWindow(int w, int h) {
    runGameRoutine([=] { ::application->window()->resize({w, h});});

    // Spontaneous events are ignored, gotta post one.
    std::unique_ptr<PlatformResizeEvent> event = std::make_unique<PlatformResizeEvent>();
    event->type = EVENT_WINDOW_RESIZE;
    event->window = ::application->window();
    event->size = {w, h};
    postEvent(std::move(event));
}

GUIButton *EngineController::existingButton(std::string_view buttonId) {
    auto findButton = [](std::string_view buttonId) -> GUIButton * {
        for (GUIWindow *window : lWindowList)
            for (GUIButton *button : window->vButtons)
                if (button->id == buttonId)
                    return button;
        return nullptr;
    };

    GUIButton *result = findButton(buttonId);
    if (!result)
        throw Exception("GUI button '{}' not found", buttonId);

    auto checkButton = [](GUIButton *button) {
        Pointi point = Pointi(button->uX + button->uWidth / 2, button->uY + button->uHeight / 2);

        for (GUIWindow *window : lWindowList) {
            for (GUIButton *otherButton : window->vButtons) {
                if (otherButton->Contains(point.x, point.y)) {
                    if (button != otherButton)
                        throw Exception("Coundn't press GUI button '{}' because it's overlapping with another GUI button", button->id);
                    return;
                }
            }
        }
    };

    checkButton(result);

    return result;
}

