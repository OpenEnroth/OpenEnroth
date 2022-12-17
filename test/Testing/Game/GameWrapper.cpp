#include "GameWrapper.h"

#include <gtest/gtest.h>

#include <cassert>
#include <utility>

#include "Platform/PlatformEvents.h"

#include "Testing/Engine/TestStateHandle.h"
#include "Testing/Engine/TestEventLoop.h"
#include "Testing/Engine/TestWindow.h"
#include "Testing/Extensions/ThrowingAssertions.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIButton.h"

GameWrapper::GameWrapper(TestStateHandle state) : state_(std::move(state)) {}

GameWrapper::~GameWrapper() {}

void GameWrapper::Tick(int count) {
    for (int i = 0; i < count; i++)
        state_.YieldExecution();
}

void GameWrapper::PressKey(PlatformKey key) {
    std::unique_ptr<PlatformKeyEvent> event = std::make_unique<PlatformKeyEvent>();
    event->type = PlatformEvent::KeyPress;
    event->key = key;
    event->mods = 0;
    event->isAutoRepeat = false;
    state_->eventLoop->PostEvent(state_->window, std::move(event));
}

void GameWrapper::ReleaseKey(PlatformKey key) {
    std::unique_ptr<PlatformKeyEvent> event = std::make_unique<PlatformKeyEvent>();
    event->type = PlatformEvent::KeyRelease;
    event->key = key;
    event->mods = 0;
    event->isAutoRepeat = false;
    state_->eventLoop->PostEvent(state_->window, std::move(event));
}

void GameWrapper::PressButton(PlatformMouseButton button, int x, int y) {
    std::unique_ptr<PlatformMouseEvent> event = std::make_unique<PlatformMouseEvent>();
    event->type = PlatformEvent::MouseButtonPress;
    event->button = PlatformMouseButton::Left;
    event->pos = Pointi(x, y);
    event->isDoubleClick = false;
    state_->eventLoop->PostEvent(state_->window, std::move(event));
}

void GameWrapper::ReleaseButton(PlatformMouseButton button, int x, int y) {
    std::unique_ptr<PlatformMouseEvent> event = std::make_unique<PlatformMouseEvent>();
    event->type = PlatformEvent::MouseButtonRelease;
    event->button = PlatformMouseButton::Left;
    event->buttons = PlatformMouseButton::Left;
    event->pos = Pointi(x, y);
    event->isDoubleClick = false;
    state_->eventLoop->PostEvent(state_->window, std::move(event));
}

void GameWrapper::PressAndReleaseKey(PlatformKey key) {
    PressKey(key);
    ReleaseKey(key);
}

void GameWrapper::PressAndReleaseButton(PlatformMouseButton button, int x, int y) {
    PressButton(button, x, y);
    ReleaseButton(button, x, y);
}

void GameWrapper::PressGuiButton(std::string_view buttonId) {
    GUIButton *button = AssertButton(buttonId);

    PressAndReleaseButton(PlatformMouseButton::Left, button->uX + button->uWidth / 2, button->uY + button->uHeight / 2);
}

void GameWrapper::GoToMainMenu() {
    if (GetCurrentMenuID() == MENU_MAIN)
        return;

    if (GetCurrentMenuID() == MENU_CREATEPARTY) {
        PressAndReleaseKey(PlatformKey::Escape);
        Tick(2);
        ASSERT_EQ(GetCurrentMenuID(), MENU_MAIN);
        return;
    }

    ASSERT_TRUE(false); // TODO(captainurist)
}


GUIButton *GameWrapper::AssertButton(std::string_view buttonId) {
    auto findButton = [](std::string_view buttonId) -> GUIButton * {
        for (GUIWindow *window : lWindowList)
            for (GUIButton *button : window->vButtons)
                if (button->id == buttonId)
                    return button;
        return nullptr;
    };

    GUIButton *result = findButton(buttonId);
    ASSERT_NE(result, nullptr) << "Button '" << buttonId << "' not found.";

    auto checkButton = [](GUIButton *button) {
        Pointi point = Pointi(button->uX + button->uWidth / 2, button->uY + button->uHeight / 2);

        for (GUIWindow *window : lWindowList) {
            for (GUIButton *otherButton : window->vButtons) {
                if (otherButton->Contains(point.x, point.y)) {
                    ASSERT_EQ(button, otherButton) << "Button '" << button->id << "' is hidden by another button.";
                    return;
                }
            }
        }
    };

    checkButton(result);

    return result;
}
