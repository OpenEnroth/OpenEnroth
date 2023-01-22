#include "EventTracer.h"

#include <memory>
#include <utility>

#include "Utility/Random/NonRandomEngine.h"
#include "Utility/Random/MersenneTwisterRandomEngine.h"
#include "Utility/Random/Random.h"

EventTracer::EventTracer() : PlatformEventFilter(PlatformEventFilter::ALL_EVENTS) {}

EventTracer::~EventTracer() {}

void EventTracer::start() {
    assert(ProxyPlatform::Base()); // Installed as a proxy.
    assert(_state == STATE_DISABLED);

    _state = STATE_WAITING;
}

void EventTracer::finish(std::string_view path) {
    assert(ProxyPlatform::Base()); // Installed as a proxy.
    assert(_state != STATE_DISABLED);

    EventTrace::saveToFile(path, _trace);
    _trace.events.clear();
    if (_oldRandomEngine)
        SetGlobalRandomEngine(std::move(_oldRandomEngine));
    _state = STATE_DISABLED;
}

int64_t EventTracer::TickCount() const {
    if (_state == STATE_TRACING) {
        return _tickCount;
    } else {
        return ProxyPlatform::TickCount();
    }
}

void EventTracer::SwapBuffers() {
    ProxyOpenGLContext::SwapBuffers();

    if (_state == STATE_DISABLED)
        return;

    if (_state == STATE_WAITING) {
        _tickCount = 0;
        _oldRandomEngine = SetGlobalRandomEngine(std::make_unique<NonRandomEngine>());
        _state = STATE_TRACING;
    }

    assert(_state == STATE_TRACING);

    PlatformEvent e;
    e.type = EventTrace::PaintEvent;

    _trace.events.emplace_back(std::make_unique<PlatformEvent>(e));
    _tickCount += 16; // Must be the same as the value in TestProxy::SwapBuffers.
}

bool EventTracer::Event(const PlatformEvent *event) {
    if (_state == STATE_TRACING)
        _trace.events.emplace_back(EventTrace::cloneEvent(event));

    return false; // We just record events & don't filter anything.
}
