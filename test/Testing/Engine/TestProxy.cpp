#include "TestProxy.h"

#include <cassert>
#include <utility>

#include "Library/Trace/EventTrace.h"

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

void TestProxy::postEvent(std::unique_ptr<PlatformEvent> event) {
    postedEvents_.push(std::move(event));
}

int64_t TestProxy::TickCount() const {
    return _tickCount;
}

void TestProxy::SwapBuffers() {
    ProxyOpenGLContext::SwapBuffers();

    _tickCount += EventTrace::FRAME_TIME_MS;

    if (_state->terminating) {
        if (_state->terminationHandler) {
            _state->terminationHandler();
            _state->terminationHandler = nullptr;
        }
    } else {
        _state.YieldExecution();
    }
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
    while (!postedEvents_.empty()) {
        std::unique_ptr<PlatformEvent> event = std::move(postedEvents_.front());
        postedEvents_.pop();
        eventHandler->Event(event.get());
    }
}
