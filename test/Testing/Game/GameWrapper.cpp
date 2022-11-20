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

void GameWrapper::Type(PlatformKey key) {
    std::unique_ptr<PlatformKeyEvent> pressEvent = std::make_unique<PlatformKeyEvent>();
    pressEvent->type = PlatformEvent::KeyPress;
    pressEvent->key = key;
    pressEvent->mods = 0;
    pressEvent->isAutoRepeat = false;

    std::unique_ptr<PlatformKeyEvent> releaseEvent = std::make_unique<PlatformKeyEvent>();
    releaseEvent->type = PlatformEvent::KeyRelease;
    releaseEvent->key = key;
    releaseEvent->mods = 0;
    releaseEvent->isAutoRepeat = false;

    state_->eventLoop->PostEvent(state_->window, std::move(pressEvent));
    state_->eventLoop->PostEvent(state_->window, std::move(releaseEvent));
}

void GameWrapper::LClick(std::string_view buttonId) {
    GUIButton *button = AssertButton(buttonId);

    LClick(button->uX + button->uWidth / 2, button->uY + button->uHeight / 2);
}

void GameWrapper::LClick(int x, int y) {
    std::unique_ptr<PlatformMouseEvent> pressEvent = std::make_unique<PlatformMouseEvent>();
    pressEvent->type = PlatformEvent::MouseButtonPress;
    pressEvent->button = PlatformMouseButton::Left;
    pressEvent->pos = Pointi(x, y);
    pressEvent->isDoubleClick = false;

    std::unique_ptr<PlatformMouseEvent> releaseEvent = std::make_unique<PlatformMouseEvent>();
    releaseEvent->type = PlatformEvent::MouseButtonRelease;
    releaseEvent->button = PlatformMouseButton::Left;
    releaseEvent->buttons = PlatformMouseButton::Left;
    releaseEvent->isDoubleClick = false;

    state_->eventLoop->PostEvent(state_->window, std::move(pressEvent));
    state_->eventLoop->PostEvent(state_->window, std::move(releaseEvent));
}

void GameWrapper::GoToMainMenu() {
    if (GetCurrentMenuID() == MENU_MAIN)
        return;

    if (GetCurrentMenuID() == MENU_CREATEPARTY) {
        Type(PlatformKey::Escape);
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
