#include "EngineTraceSimplePlayer.h"

#include <stdint.h>
#include <cassert>
#include <utility>

#include "Engine/Components/Control/EngineController.h"
#include "Library/Application/PlatformApplication.h"
#include "Library/Trace/PaintEvent.h"
#include "Library/Random/Random.h"
#include "Utility/ScopeGuard.h"
#include "Utility/Exception.h"
#include "Engine/Components/Trace/EngineTraceEnums.h"
#include "Library/Random/RandomEngine.h"
#include "Platform/Platform.h"
#include "Platform/PlatformEvents.h"
#include "Utility/Flags.h"
#include "fmt/core.h"

EngineTraceSimplePlayer::EngineTraceSimplePlayer() = default;
EngineTraceSimplePlayer::~EngineTraceSimplePlayer() = default;

void EngineTraceSimplePlayer::playTrace(EngineController *game, std::vector<std::unique_ptr<PlatformEvent>> events,
                                        const std::string &tracePath, EngineTracePlaybackFlags flags, std::function<void()> tickCallback) {
    assert(!isPlaying());

    _playing = true;
    MM_AT_SCOPE_EXIT(_playing = false);

    _tracePath = tracePath;
    _flags = flags;

    if (tickCallback)
        tickCallback();

    for (std::unique_ptr<PlatformEvent> &event : events) {
        if (event->type == EVENT_PAINT) {
            game->tick(1);

            if (tickCallback)
                tickCallback();

            const PaintEvent *paintEvent = static_cast<const PaintEvent *>(event.get());
            checkTime(paintEvent);
            checkRng(paintEvent);
        } else {
            game->postEvent(std::move(event));
        }
    }

    if (tickCallback)
        tickCallback();
}

void EngineTraceSimplePlayer::checkTime(const PaintEvent *paintEvent) {
    if (_flags & TRACE_PLAYBACK_SKIP_TIME_CHECKS)
        return;

    int64_t tickCount = application()->platform()->tickCount();
    if (tickCount != paintEvent->tickCount) {
        throw Exception("Tick count desynchronized when playing back trace '{}': expected {}, got {}",
                        _tracePath, paintEvent->tickCount, tickCount);
    }
}

void EngineTraceSimplePlayer::checkRng(const PaintEvent *paintEvent) {
    if (_flags & TRACE_PLAYBACK_SKIP_RANDOM_CHECKS)
        return;

    int randomState = grng->peek(1024);
    int64_t tickCount = application()->platform()->tickCount();
    if (randomState != paintEvent->randomState) {
        throw Exception("Random state desynchronized when playing back trace '{}' at {}ms: expected {}, got {}",
                        _tracePath, tickCount, paintEvent->randomState, randomState);
    }
}
