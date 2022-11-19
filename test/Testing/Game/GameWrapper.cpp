#include "GameWrapper.h"

#include <cassert>
#include <utility>

#include "Platform/PlatformEvents.h"

#include "GameTestStateHandle.h"
#include "GameTestEventLoop.h"
#include "GameTestWindow.h"

GameWrapper::GameWrapper(GameTestStateHandle state) : state_(std::move(state)) {}

GameWrapper::~GameWrapper() {}

void GameWrapper::Tick(int count) {
    for (int i = 0; i < count; i++)
        state_.SwitchSide();
}

void GameWrapper::LClick(int x, int y) {
    std::unique_ptr<PlatformMouseEvent> clickEvent = std::make_unique<PlatformMouseEvent>();
    clickEvent->type = PlatformEvent::MouseButtonPress;
    clickEvent->button = PlatformMouseButton::Left;
    clickEvent->pos = Pointi(x, y);
    clickEvent->isDoubleClick = false;

    std::unique_ptr<PlatformMouseEvent> releaseEvent = std::make_unique<PlatformMouseEvent>();
    releaseEvent->type = PlatformEvent::MouseButtonRelease;
    releaseEvent->button = PlatformMouseButton::Left;
    releaseEvent->buttons = PlatformMouseButton::Left;
    releaseEvent->isDoubleClick = false;

    state_->eventLoop->PostEvent(state_->window, std::move(clickEvent));
    state_->eventLoop->PostEvent(state_->window, std::move(releaseEvent));
}
