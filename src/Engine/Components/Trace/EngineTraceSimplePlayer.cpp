#include "EngineTraceSimplePlayer.h"

#include <cassert>
#include <utility>
#include <vector>
#include <memory>

#include "Engine/Components/Control/EngineController.h"
#include "Engine/Random/Random.h"

#include "Library/Platform/Application/PlatformApplication.h"
#include "Library/Trace/PaintEvent.h"

#include "Utility/ScopeGuard.h"
#include "Utility/Exception.h"

EngineTraceSimplePlayer::EngineTraceSimplePlayer() = default;
EngineTraceSimplePlayer::~EngineTraceSimplePlayer() = default;

void EngineTraceSimplePlayer::playTrace(EngineController *game, std::vector<std::unique_ptr<PlatformEvent>> events,
                                        std::string_view traceDisplayPath, EngineTracePlaybackFlags flags) {
    assert(!isPlaying());

    _playing = true;
    MM_AT_SCOPE_EXIT(_playing = false);

    _traceDisplayPath = traceDisplayPath;
    _flags = flags;

    for (std::unique_ptr<PlatformEvent> &event : events) {
        if (event->type == EVENT_PAINT) {
            game->tick(1);

            const PaintEvent *paintEvent = static_cast<const PaintEvent *>(event.get());
            checkTime(paintEvent);
            checkRng(paintEvent);
        } else {
            game->postEvent(std::move(event));
        }
    }
}

void EngineTraceSimplePlayer::checkTime(const PaintEvent *paintEvent) {
    if (_flags & TRACE_PLAYBACK_SKIP_TIME_CHECKS)
        return;

    int64_t tickCount = application()->platform()->tickCount();
    if (tickCount != paintEvent->tickCount) {
        throw Exception("Tick count desynchronized when playing back trace '{}': expected {}, got {}",
                        _traceDisplayPath, paintEvent->tickCount, tickCount);
    }
}

void EngineTraceSimplePlayer::checkRng(const PaintEvent *paintEvent) {
    if (_flags & TRACE_PLAYBACK_SKIP_RANDOM_CHECKS)
        return;

    int randomState = grng->peek(1024 * 1024);
    int64_t tickCount = application()->platform()->tickCount();
    if (randomState != paintEvent->randomState) {
        throw Exception("Random state desynchronized when playing back trace '{}' at {}ms: expected {}, got {}",
                        _traceDisplayPath, tickCount, paintEvent->randomState, randomState);
    }
}
