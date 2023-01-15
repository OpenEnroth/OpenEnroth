#include "TestProxy.h"

#include <cassert>
#include <utility>

#include "Platform/PlatformEventHandler.h"

TestProxy::TestProxy(TestStateHandle state) : _state(state) {
    assert(!_state->proxy);
    _state->proxy = this;

    emptyHandler_ = std::make_unique<PlatformEventHandler>();
}

TestProxy::~TestProxy() {
    assert(_state->proxy == this);
    _state->proxy = nullptr;
}

void TestProxy::reset() {
    _tickCount = 0;
}

void TestProxy::postEvent(PlatformWindow *window, std::unique_ptr<PlatformEvent> event) {
    postedEvents_.push({window, std::move(event)});
}

int64_t TestProxy::TickCount() const {
    return _tickCount;
}

void TestProxy::SwapBuffers() {
    ProxyOpenGLContext::SwapBuffers();

    if (_state->terminating) {
        if (_state->terminationHandler) {
            _state->terminationHandler();
            _state->terminationHandler = nullptr;
        }
    } else {
        _state.YieldExecution();
    }

    // 16ms translates to 62.5fps.
    // It is possible to use double for state here (or store microseconds / nanoseconds), but this will result in
    // staggered frame times, with every 1st and 2nd frame taking 17ms, and every 3rd one taking 16ms.
    // This might result in some non-determinism down the line, e.g. changing the code in level loading will change
    // the number of frames it takes to load a level, and this will shift the timing sequence for the actual game
    // frames after the level is loaded. Unlikely to really affect anything, but we'd rather not find out.
    _tickCount += 16;
}

void TestProxy::Exec(PlatformEventHandler *eventHandler) {
    processSyntheticMessages(eventHandler);
    ProxyEventLoop::Exec(emptyHandler_.get());
}

void TestProxy::ProcessMessages(PlatformEventHandler *eventHandler, int count) {
    processSyntheticMessages(eventHandler);
    ProxyEventLoop::ProcessMessages(emptyHandler_.get(), count);
}

void TestProxy::processSyntheticMessages(PlatformEventHandler *eventHandler) {
    while(!postedEvents_.empty()) {
        PostedEvent event = std::move(postedEvents_.front());
        postedEvents_.pop();
        eventHandler->Event(event.window, event.event.get());
    }
}
