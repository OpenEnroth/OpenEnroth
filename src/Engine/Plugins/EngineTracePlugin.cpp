#include "EngineTracePlugin.h"

#include <cassert>
#include <utility>
#include <memory>

#include "Library/Application/PlatformApplication.h"
#include "Library/Random/Random.h"
#include "Library/Trace/PaintEvent.h"

EngineTracePlugin::EngineTracePlugin(): PlatformEventFilter(EVENTS_ALL) {}
EngineTracePlugin::~EngineTracePlugin() = default;

void EngineTracePlugin::start() {
    assert(!_tracing);
    _tracing = true;
}

EventTrace EngineTracePlugin::finish() {
    assert(_tracing);
    _tracing = false;
    return std::move(_trace);
}

void EngineTracePlugin::swapBuffers() {
    if (_tracing) {
        std::unique_ptr<PaintEvent> e = std::make_unique<PaintEvent>();
        e->type = EVENT_PAINT;
        e->tickCount = application()->platform()->tickCount();
        e->randomState = grng->Random(1024);
        _trace.events.push_back(std::move(e));
    }

    // Tail calling is good practice - this way users can reason about the order of proxy execution.
    ProxyOpenGLContext::swapBuffers();
}

bool EngineTracePlugin::event(const PlatformEvent *event) {
    if (_tracing && EventTrace::isTraceable(event))
        _trace.events.push_back(EventTrace::cloneEvent(event));
    return false;
}
