#include "EventTracer.h"

#include <memory>
#include <utility>
#include <filesystem>

#include "Library/Random/NonRandomEngine.h"
#include "Library/Random/Random.h"
#include "Utility/DataPath.h"

#include "Engine/SaveLoad.h" // TODO(captainurist): EventTracer now belongs in Engine, not in Library

#include "PaintEvent.h"

EventTracer::EventTracer() : PlatformEventFilter(PlatformEventFilter::ALL_EVENTS) {}

EventTracer::~EventTracer() {}

void EventTracer::start(const std::string &tracePath, const std::string &savePath) {
    assert(ProxyPlatform::Base()); // Installed as a proxy.
    assert(_state == STATE_DISABLED);

    _state = STATE_WAITING;
    _tracePath = tracePath;
    _savePath = savePath;
}

void EventTracer::finish() {
    assert(ProxyPlatform::Base()); // Installed as a proxy.
    assert(_state != STATE_DISABLED);

    EventTrace::saveToFile(_tracePath, _trace);
    _trace.events.clear();
    if (_oldRandomEngine)
        grng = std::move(_oldRandomEngine);
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
    case STATE_WAITING: {
        SaveGame(true, false);

        std::error_code ec;
        std::string src = MakeDataPath("saves", "autosave.mm7");
        std::filesystem::copy_file(src, _savePath, std::filesystem::copy_options::overwrite_existing, ec);
        // TODO(captainurist): at least log on error

        _tickCount = 0;
        _state = STATE_TRACING;
        _oldRandomEngine = std::move(grng);
        grng = std::make_unique<NonRandomEngine>();
        break;
    }
    case STATE_TRACING:
        _tickCount += EventTrace::FRAME_TIME_MS;
        break;
    }

    assert(_state == STATE_TRACING);

    PaintEvent e;
    e.type = PaintEvent::Paint;
    e.tickCount = _tickCount;
    e.randomState = grng->Random(1024);

    _trace.events.emplace_back(std::make_unique<PaintEvent>(e));
}

bool EventTracer::Event(const PlatformEvent *event) {
    if (_state == STATE_TRACING)
        _trace.events.emplace_back(EventTrace::cloneEvent(event));

    return false; // We just record events & don't filter anything.
}
