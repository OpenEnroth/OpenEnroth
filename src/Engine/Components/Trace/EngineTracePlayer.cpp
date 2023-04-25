#include "EngineTracePlayer.h"

#include <cassert>
#include <utility>
#include <filesystem>
#include <memory>

#include "Application/GameKeyboardController.h" // TODO(captainurist): Engine -> Application dependency

#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Deterministic/EngineDeterministicComponent.h"
#include "Engine/Engine.h"

#include "Library/Trace/PaintEvent.h"
#include "Library/Trace/EventTrace.h"
#include "Library/Application/PlatformApplication.h"
#include "Library/Random/Random.h"

#include "Utility/ScopeGuard.h"
#include "Utility/Exception.h"

#include "EngineTraceComponent.h"
#include "EngineTraceConfigurator.h"

EngineTracePlayer::EngineTracePlayer() {}

EngineTracePlayer::~EngineTracePlayer() {
    assert(!application()); // We're uninstalled.
}

void EngineTracePlayer::playTrace(EngineController *game, const std::string &savePath, const std::string &tracePath,
                                  EngineTracePlaybackFlags flags, std::function<void()> postLoadCallback) {
    prepareTrace(game, savePath, tracePath);

    if (postLoadCallback)
        postLoadCallback();

    playPreparedTrace(game, flags);
}

void EngineTracePlayer::prepareTrace(EngineController *game, const std::string &savePath, const std::string &tracePath) {
    assert(!_trace);

    std::unique_ptr<EventTrace> trace = std::make_unique<EventTrace>(EventTrace::loadFromFile(tracePath, application()->window()));

    int saveFileSize = std::filesystem::file_size(savePath);
    if (trace->header.saveFileSize != -1 && trace->header.saveFileSize != saveFileSize) {
        throw Exception("Trace '{}' expected a savegame of size {} bytes, but the size of '{}' is {} bytes",
                        tracePath, trace->header.saveFileSize, savePath, saveFileSize);
    }

    game->resizeWindow(640, 480);
    game->tick();

    EngineTraceConfigurator::patchConfig(engine->config.get(), trace->header.config);
    int frameTimeMs = engine->config->debug.TraceFrameTimeMs.value();

    _deterministicComponent->startDeterministicSegment(frameTimeMs);
    game->loadGame(savePath);
    _deterministicComponent->startDeterministicSegment(frameTimeMs);
    _keyboardController->reset(); // Reset all pressed buttons.

    _tracePath = tracePath;
    _trace = std::move(trace);
}

void EngineTracePlayer::playPreparedTrace(EngineController *game, EngineTracePlaybackFlags flags) {
    assert(_trace);

    MM_AT_SCOPE_EXIT({
        _deterministicComponent->finish();
        _trace.reset();
        _tracePath.clear();
    });

    for (std::unique_ptr<PlatformEvent> &event : _trace->events) {
        if (event->type == EVENT_PAINT) {
            game->tick(1);

            const PaintEvent *paintEvent = static_cast<const PaintEvent *>(event.get());

            int64_t tickCount = application()->platform()->tickCount();
            if (!(flags & TRACE_PLAYBACK_SKIP_TIME_CHECKS) && tickCount != paintEvent->tickCount) {
                throw Exception("Tick count desynchronized when playing back trace '{}': expected {}, got {}",
                                _tracePath, paintEvent->tickCount, tickCount);
            }

            int randomState = grng->peek(1024);
            if (!(flags & TRACE_PLAYBACK_SKIP_RANDOM_CHECKS) && randomState != paintEvent->randomState) {
                throw Exception("Random state desynchronized when playing back trace '{}' at {}ms: expected {}, got {}",
                                _tracePath, tickCount, paintEvent->randomState, randomState);
            }
        } else {
            game->postEvent(std::move(event));
        }
    }
}

void EngineTracePlayer::installNotify() {
    _deterministicComponent = application()->get<EngineDeterministicComponent>();
    _keyboardController = application()->get<GameKeyboardController>();
}

void EngineTracePlayer::removeNotify() {
    _deterministicComponent = nullptr;
    _keyboardController = nullptr;
}
