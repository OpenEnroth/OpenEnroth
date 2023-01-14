#include "TestEventLoop.h"

#include <cassert>
#include <utility>

#include "Platform/PlatformEventHandler.h"

#include "TestStateHandle.h"

TestEventLoop::TestEventLoop(std::unique_ptr<PlatformEventLoop> base, TestStateHandle state) :
    ProxyEventLoop(base.get()),
    base_(std::move(base)),
    state_(std::move(state)) {
    assert(!state_->eventLoop);
    state_->eventLoop = this;
    emptyHandler_ = std::make_unique<PlatformEventHandler>();
}

TestEventLoop::~TestEventLoop() {
    assert(state_->eventLoop == this);
    state_->eventLoop = nullptr;
}

void TestEventLoop::Exec(PlatformEventHandler *eventHandler) {
    ProcessSyntheticMessages(eventHandler);
    base_->Exec(emptyHandler_.get());
}

void TestEventLoop::ProcessMessages(PlatformEventHandler *eventHandler, int count) {
    ProcessSyntheticMessages(eventHandler);
    base_->ProcessMessages(emptyHandler_.get(), count);
}

void TestEventLoop::PostEvent(PlatformWindow *window, std::unique_ptr<PlatformEvent> event) {
    postedEvents_.push({window, std::move(event)});
}

void TestEventLoop::ProcessSyntheticMessages(PlatformEventHandler *eventHandler) {
    while(!postedEvents_.empty()) {
        PostedEvent &e = postedEvents_.front();
        eventHandler->Event(e.window, e.event.get());
        postedEvents_.pop();
    }
}

