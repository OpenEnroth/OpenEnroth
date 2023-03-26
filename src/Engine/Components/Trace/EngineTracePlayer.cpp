#include "EngineTracePlayer.h"

#include <cassert>
#include <utility>
#include <filesystem>
#include <memory>

#include "Application/GameKeyboardController.h" // TODO(captainurist): Engine -> Application dependency

#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Deterministic/EngineDeterministicComponent.h"

#include "Library/Trace/PaintEvent.h"
#include "Library/Application/PlatformApplication.h"
#include "Library/Random/Random.h"

#include "Utility/ScopeGuard.h"
#include "Utility/Exception.h"

#include "EngineTraceComponent.h"

EngineTracePlayer::~EngineTracePlayer() {
    assert(!application()); // We're uninstalled.
}

void EngineTracePlayer::playTrace(EngineController *game, const std::string &savePath, const std::string &tracePath,
                                  EngineTracePlaybackFlags flags, std::function<void()> postLoadCallback) {
    assert(!isPlaying());

    _isPlaying = true;
    MM_AT_SCOPE_EXIT(_isPlaying = false);

    EventTrace trace = EventTrace::loadFromFile(tracePath, application()->window());

    int saveFileSize = std::filesystem::file_size(savePath);
    if (trace.header.saveFileSize != -1 && trace.header.saveFileSize != saveFileSize) {
        throw Exception("Trace '{}' expected a savegame of size {} bytes, but the size of '{}' is {} bytes",
                        tracePath, trace.header.saveFileSize, savePath, saveFileSize);
    }

    game->resizeWindow(640, 480);
    game->tick();

    _deterministicComponent->enterDeterministicMode();
    MM_AT_SCOPE_EXIT(_deterministicComponent->leaveDeterministicMode());

    game->loadGame(savePath);
    _deterministicComponent->resetDeterministicState();
    _keyboardController->reset(); // Reset all pressed buttons.

    if (postLoadCallback)
        postLoadCallback();

    for (std::unique_ptr<PlatformEvent> &event : trace.events) {
        if (event->type == EVENT_PAINT) {
            game->tick(1);

            const PaintEvent *paintEvent = static_cast<const PaintEvent *>(event.get());

            int64_t tickCount = application()->platform()->tickCount();
            if (!(flags & TRACE_PLAYBACK_SKIP_TIME_CHECKS) && tickCount != paintEvent->tickCount) {
                throw Exception("Tick count desynchronized when playing back trace '{}': expected {}, got {}",
                                tracePath, paintEvent->tickCount, tickCount);
            }

            int randomState = grng->Random(1024);
            if (!(flags & TRACE_PLAYBACK_SKIP_RANDOM_CHECKS) && randomState != paintEvent->randomState) {
                throw Exception("Random state desynchronized when playing back trace '{}' at {}ms: expected {}, got {}",
                                tracePath, tickCount, paintEvent->randomState, randomState);
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


