#include "EngineTraceSimpleRecorder.h"

#include <cassert>
#include <utility>
#include <memory>

#include "Library/Platform/Application/PlatformApplication.h"
#include "Library/Random/Random.h"
#include "Library/Trace/PaintEvent.h"
#include "Library/Trace/EventTrace.h"

EngineTraceSimpleRecorder::EngineTraceSimpleRecorder(): PlatformEventFilter(EVENTS_ALL) {}
EngineTraceSimpleRecorder::~EngineTraceSimpleRecorder() = default;

void EngineTraceSimpleRecorder::startRecording() {
    assert(!isRecording());

    _recording = true;
    _events.clear();
}

std::vector<std::unique_ptr<PlatformEvent>> EngineTraceSimpleRecorder::finishRecording() {
    assert(isRecording());

    _recording = false;
    return std::move(_events);
}

void EngineTraceSimpleRecorder::swapBuffers() {
    if (isRecording()) {
        std::unique_ptr<PaintEvent> e = std::make_unique<PaintEvent>();
        e->type = EVENT_PAINT;
        e->tickCount = application()->platform()->tickCount();
        e->randomState = grng->peek(1024);
        _events.push_back(std::move(e));
    }

    ProxyOpenGLContext::swapBuffers();
}

bool EngineTraceSimpleRecorder::event(const PlatformEvent *event) {
    if (isRecording() && EventTrace::isTraceable(event))
        _events.push_back(EventTrace::cloneEvent(event));
    return false;
}
