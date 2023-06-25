#include "EngineTraceComponent.h"

#include <cassert>
#include <utility>
#include <memory>

#include "Engine/Engine.h"

#include "Library/Application/PlatformApplication.h"
#include "Library/Random/Random.h"
#include "Library/Trace/PaintEvent.h"
#include "Library/Trace/EventTrace.h"

#include "EngineTraceStateAccessor.h"

EngineTraceComponent::EngineTraceComponent(): PlatformEventFilter(EVENTS_ALL) {}
EngineTraceComponent::~EngineTraceComponent() = default;

void EngineTraceComponent::startRecording() {
    assert(!_trace);

    _trace = std::make_unique<EventTrace>();
    _trace->header.config = EngineTraceStateAccessor::makeConfigPatch(engine->config.get());
    _trace->header.startState = EngineTraceStateAccessor::makeGameState();
}

EventTrace EngineTraceComponent::finishRecording() {
    assert(_trace);

    _trace->header.endState = EngineTraceStateAccessor::makeGameState();

    EventTrace result = std::move(*_trace);
    _trace.reset();
    return result;
}

void EngineTraceComponent::swapBuffers() {
    if (_trace) {
        std::unique_ptr<PaintEvent> e = std::make_unique<PaintEvent>();
        e->type = EVENT_PAINT;
        e->tickCount = application()->platform()->tickCount();
        e->randomState = grng->peek(1024);
        _trace->events.push_back(std::move(e));
    }

    // Tail calling is good practice - this way users can reason about the order of proxy execution.
    ProxyOpenGLContext::swapBuffers();
}

bool EngineTraceComponent::event(const PlatformEvent *event) {
    if (_trace && EventTrace::isTraceable(event))
        _trace->events.push_back(EventTrace::cloneEvent(event));
    return false;
}
