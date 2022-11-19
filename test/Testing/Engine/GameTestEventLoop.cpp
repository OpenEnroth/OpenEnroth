#include "GameTestEventLoop.h"

#include <cassert>
#include <utility>

#include "Platform/PlatformEventHandler.h"

#include "GameTestStateHandle.h"

GameTestEventLoop::GameTestEventLoop(std::unique_ptr<PlatformEventLoop> base, GameTestStateHandle state) :
    base_(std::move(base)),
    state_(std::move(state)) {
    assert(!state_->eventLoop);
    state_->eventLoop = this;
}

GameTestEventLoop::~GameTestEventLoop() {
    assert(state_->eventLoop == this);
    state_->eventLoop = nullptr;
}

void GameTestEventLoop::Exec(PlatformEventHandler *eventHandler) {
    ProcessSyntheticMessages(eventHandler);
    base_->Exec(eventHandler);
}

void GameTestEventLoop::Quit() {
    base_->Quit();
}

void GameTestEventLoop::ProcessMessages(PlatformEventHandler *eventHandler, int count) {
    ProcessSyntheticMessages(eventHandler);
    base_->ProcessMessages(eventHandler, count);
}

void GameTestEventLoop::WaitForMessages() {
    base_->WaitForMessages();
}

void GameTestEventLoop::PostEvent(PlatformWindow *window, std::unique_ptr<PlatformEvent> event) {
    postedEvents_.push({window, std::move(event)});
}

void GameTestEventLoop::ProcessSyntheticMessages(PlatformEventHandler *eventHandler) {
    while(!postedEvents_.empty()) {
        PostedEvent &e = postedEvents_.front();
        eventHandler->Event(e.window, e.event.get());
        postedEvents_.pop();
    }
}

