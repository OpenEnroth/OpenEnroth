#include "EngineTracePlugin.h"

#include <cassert>
#include <utility>
#include <memory>

#include "Library/Application/PlatformApplication.h"
#include "Library/Random/Random.h"
#include "Library/Trace/PaintEvent.h"

EngineTracePlugin::EngineTracePlugin(): PlatformEventFilter(PlatformEventFilter::ALL_EVENTS) {}
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

void EngineTracePlugin::SwapBuffers() {
    if (_tracing) {
        std::unique_ptr<PaintEvent> e = std::make_unique<PaintEvent>();
        e->type = PaintEvent::Paint;
        e->tickCount = application()->platform()->TickCount();
        e->randomState = grng->Random(1024);
        _trace.events.push_back(std::move(e));
    }

    // Tail calling is good practice - this way users can reason about the order of proxy execution.
    ProxyOpenGLContext::SwapBuffers();
}

bool EngineTracePlugin::Event(const PlatformEvent *event) {
    if (_tracing && EventTrace::isTraceable(event))
        _trace.events.push_back(EventTrace::cloneEvent(event));
    return false;
}
