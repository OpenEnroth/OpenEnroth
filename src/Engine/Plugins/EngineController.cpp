#include "EngineController.h"

#include <cassert>
#include <stdexcept>
#include <filesystem>
#include <utility>
#include <thread>

#include "Arcomage/Arcomage.h"

#include "GUI/GUIProgressBar.h"
#include "GUI/GUIWindow.h"
#include "GUI/GUIButton.h"

#include "Engine/SaveLoad.h"
#include "Engine/EngineGlobals.h"

#include "Library/Application/PlatformApplication.h"

#include "Platform/PlatformEvents.h"

#include "Utility/Format.h"

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
    event->type = PlatformEvent::KeyPress;
    event->window = ::application->window();
    event->key = key;
    event->mods = 0;
    event->isAutoRepeat = false;
    postEvent(std::move(event));
}

void EngineController::releaseKey(PlatformKey key) {
    std::unique_ptr<PlatformKeyEvent> event = std::make_unique<PlatformKeyEvent>();
    event->type = PlatformEvent::KeyRelease;
    event->window = ::application->window();
    event->key = key;
    event->mods = 0;
    event->isAutoRepeat = false;
    postEvent(std::move(event));
}

void EngineController::pressButton(PlatformMouseButton button, int x, int y) {
    std::unique_ptr<PlatformMouseEvent> event = std::make_unique<PlatformMouseEvent>();
    event->type = PlatformEvent::MouseButtonPress;
    event->window = ::application->window();
    event->button = PlatformMouseButton::Left;
    event->pos = Pointi(x, y);
    event->isDoubleClick = false;
    postEvent(std::move(event));
}

void EngineController::releaseButton(PlatformMouseButton button, int x, int y) {
    std::unique_ptr<PlatformMouseEvent> event = std::make_unique<PlatformMouseEvent>();
    event->type = PlatformEvent::MouseButtonRelease;
    event->window = ::application->window();
    event->button = PlatformMouseButton::Left;
    event->buttons = PlatformMouseButton::Left;
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

    pressAndReleaseButton(PlatformMouseButton::Left, button->uX + button->uWidth / 2, button->uY + button->uHeight / 2);
}

void EngineController::goToMainMenu() {
    if (GetCurrentMenuID() == MENU_MAIN)
        return;

    while (pArcomageGame->bGameInProgress) {
        pressAndReleaseKey(PlatformKey::Escape);
        tick(1);
    }

    while (current_screen_type == CURRENT_SCREEN::SCREEN_HOUSE) {
        pressAndReleaseKey(PlatformKey::Escape);
        tick(1);
    }

    if (GetCurrentMenuID() == MENU_CREATEPARTY) {
        pressAndReleaseKey(PlatformKey::Escape);
        tick(2);
        assert(GetCurrentMenuID() == MENU_MAIN);
        return;
    }

    if (GetCurrentMenuID() == MENU_NONE) {
        pressAndReleaseKey(PlatformKey::Escape);
        tick(1);
        pressGuiButton("GameMenu_Quit");
        tick(1);
        pressGuiButton("GameMenu_Quit");
        while (GetCurrentMenuID() != MENU_MAIN)
            tick(1);
        return;
    }

    assert(false); // TODO(captainurist): implement for other cases
}

void EngineController::skipLoadingScreen() {
    int steps = 0;
    while (!pGameLoadingUI_ProgressBar->IsActive()) {
        tick(1);
        steps++;
        if (steps >= 128)
            throw std::runtime_error("Can't skip a non-existent loading screen");
    }
    while (pGameLoadingUI_ProgressBar->IsActive())
        tick(1);
    while (dword_6BE364_game_settings_1 & GAME_SETTINGS_0080_SKIP_USER_INPUT_THIS_FRAME)
        tick(1);
}

void EngineController::saveGame(const std::string &path) {
    // SaveGame makes a screenshot and needs the opengl context that's bound in game thread, so we cannot call it from
    // the control thread. One option is to unbind every time we switch to control thread, but this is slow, and not
    // needed 99% of the time. So we just call back into the game thread.
    runGameRoutine([] { ::SaveGame(true, false); });

    std::string src = MakeDataPath("saves", "autosave.mm7");
    std::filesystem::copy_file(src, path, std::filesystem::copy_options::overwrite_existing); // This might throw.
}

void EngineController::loadGame(const std::string &path) {
    std::string saveName = "!!!test.mm7";
    std::string dst = MakeDataPath("saves", saveName);
    std::filesystem::copy_file(path, dst, std::filesystem::copy_options::overwrite_existing); // This might throw.

    goToMainMenu();
    pressGuiButton("MainMenu_LoadGame");
    tick(3);

    // TODO(captainurist): the tricks above might fail if we have more than 45 save files
    assert(pSavegameUsedSlots[0]);
    assert(pSavegameList->pFileList[0] == saveName);

    pressGuiButton("LoadMenu_Slot0");
    tick(2);
    pressGuiButton("LoadMenu_Load");
    skipLoadingScreen();
}

void EngineController::runGameRoutine(GameRoutine routine) {
    _state->gameRoutine = std::move(routine);
    _state.yieldExecution();
    assert(!_state->gameRoutine); // Must have finished.
    assert(!_state->terminating); // Please don't do anything crazy in the game routine.
}

void EngineController::resizeWindow(int w, int h) {
    runGameRoutine([=] { ::application->window()->Resize({w, h});});

    // Spontaneous events are ignored, gotta post one.
    std::unique_ptr<PlatformResizeEvent> event = std::make_unique<PlatformResizeEvent>();
    event->type = PlatformEvent::WindowResize;
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
        throw std::runtime_error(fmt::format("GUI button '{}' not found", buttonId));

    auto checkButton = [](GUIButton *button) {
        Pointi point = Pointi(button->uX + button->uWidth / 2, button->uY + button->uHeight / 2);

        for (GUIWindow *window : lWindowList) {
            for (GUIButton *otherButton : window->vButtons) {
                if (otherButton->Contains(point.x, point.y)) {
                    if (button != otherButton)
                        throw std::runtime_error(fmt::format("Coundn't press GUI button '{}' because it's overlapping with another GUI button", button->id));
                    return;
                }
            }
        }
    };

    checkButton(result);

    return result;
}

