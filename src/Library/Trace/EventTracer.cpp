#include "EventTracer.h"

#include <memory>
#include <utility>

#include "Utility/Random/NonRandomEngine.h"
#include "Utility/Random/MersenneTwisterRandomEngine.h"
#include "Utility/Random/Random.h"

#include "PaintEvent.h"

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

    switch (_state) {
    case STATE_DISABLED:
        return;
    case STATE_WAITING:
        _tickCount = 0;
        _oldRandomEngine = SetGlobalRandomEngine(std::make_unique<NonRandomEngine>());
        _state = STATE_TRACING;
        break;
    case STATE_TRACING:
        _tickCount += EventTrace::FRAME_TIME_MS;
        break;
    }

    assert(_state == STATE_TRACING);

    PaintEvent e;
    e.type = PaintEvent::Paint;
    e.tickCount = _tickCount;
    e.randomState = Random(1024);

    _trace.events.emplace_back(std::make_unique<PaintEvent>(e));
}

bool EventTracer::Event(const PlatformEvent *event) {
    if (_state == STATE_TRACING)
        _trace.events.emplace_back(EventTrace::cloneEvent(event));

    return false; // We just record events & don't filter anything.
}
