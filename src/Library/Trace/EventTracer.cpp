#include "EventTracer.h"

#include <memory>
#include <utility>

#include "Utility/Random/NonRandomEngine.h"
#include "Utility/Random/MersenneTwisterRandomEngine.h"
#include "Utility/Random/Random.h"

EventTracer::EventTracer() : PlatformEventFilter(PlatformEventFilter::ALL_EVENTS) {}

EventTracer::~EventTracer() {}

void EventTracer::start() {
    assert(ProxyPlatform::Base() && !_tracing); // Proxies are installed && not already tracing

    // TODO(captainurist): tracing should start inside SwapBuffers() call, because that's where playback starts.
    _tickCount = 0;
    _oldRandomEngine = SetGlobalRandomEngine(std::make_unique<NonRandomEngine>());
    _tracing = true;
}

void EventTracer::finish(std::string_view path) {
    assert(ProxyPlatform::Base() && _tracing); // Proxies are installed && is tracing

    EventTrace::saveToFile(path, _trace);
    _trace.events.clear();
    SetGlobalRandomEngine(std::move(_oldRandomEngine));
    _tracing = false;
}

int64_t EventTracer::TickCount() const {
    if (_tracing) {
        return _tickCount;
    } else {
        return ProxyPlatform::TickCount();
    }
}

void EventTracer::SwapBuffers() {
    ProxyOpenGLContext::SwapBuffers();

    if (_tracing) {
        PlatformEvent e;
        e.type = EventTrace::PaintEvent;

        _trace.events.emplace_back(std::make_unique<PlatformEvent>(e));
        _tickCount += 16; // Must be the same as the value in TestProxy::SwapBuffers.
    }
}

bool EventTracer::Event(const PlatformEvent *event) {
    if (_tracing)
        _trace.events.emplace_back(EventTrace::cloneEvent(event));

    return false; // We just record events & don't filter anything.
}
