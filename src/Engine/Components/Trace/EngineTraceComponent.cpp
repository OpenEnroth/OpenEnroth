#include "EngineTraceComponent.h"

#include <cassert>
#include <utility>
#include <memory>

#include "Library/Application/PlatformApplication.h"
#include "Library/Random/Random.h"
#include "Library/Trace/PaintEvent.h"
#include "Library/Trace/EventTrace.h"

EngineTraceComponent::EngineTraceComponent(): PlatformEventFilter(EVENTS_ALL) {}
EngineTraceComponent::~EngineTraceComponent() = default;

void EngineTraceComponent::start() {
    assert(!_tracing);
    _tracing = true;
}

std::vector<std::unique_ptr<PlatformEvent>> EngineTraceComponent::finish() {
    assert(_tracing);
    _tracing = false;
    return std::move(_trace);
}

void EngineTraceComponent::swapBuffers() {
    if (_tracing) {
        std::unique_ptr<PaintEvent> e = std::make_unique<PaintEvent>();
        e->type = EVENT_PAINT;
        e->tickCount = application()->platform()->tickCount();
        e->randomState = grng->peek(1024);
        _trace.push_back(std::move(e));
    }

    // Tail calling is good practice - this way users can reason about the order of proxy execution.
    ProxyOpenGLContext::swapBuffers();
}

bool EngineTraceComponent::event(const PlatformEvent *event) {
    if (_tracing && EventTrace::isTraceable(event))
        _trace.push_back(EventTrace::cloneEvent(event));
    return false;
}
