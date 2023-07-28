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

#include "EngineTraceStateAccessor.h"
#include "EngineTraceSimplePlayer.h"

EngineTracePlayer::EngineTracePlayer() {}

EngineTracePlayer::~EngineTracePlayer() {
    assert(!application()); // We're uninstalled.
}

void EngineTracePlayer::playTrace(EngineController *game, const std::string &savePath, const std::string &tracePath,
                                  EngineTracePlaybackFlags flags, std::function<void()> postLoadCallback,
                                  std::function<void()> tickCallback) {
    assert(!isPlaying());
    EngineDeterministicComponent *deterministicComponent = application()->get<EngineDeterministicComponent>();

    _tracePath = tracePath;
    _savePath = savePath;
    _flags = flags;
    _trace = std::make_unique<EventTrace>(EventTrace::loadFromFile(_tracePath, application()->window()));

    MM_AT_SCOPE_EXIT({
        _tracePath.clear();
        _savePath.clear();
        _flags = 0;
        _trace.reset();
        deterministicComponent->finish();
    });

    checkSaveFileSize(_trace->header.saveFileSize);

    game->resizeWindow(640, 480);
    game->tick();

    EngineTraceStateAccessor::patchConfig(engine->config.get(), _trace->header.config);
    int frameTimeMs = engine->config->debug.TraceFrameTimeMs.value();

    game->goToMainMenu(); // This might call into a random engine.
    deterministicComponent->restart(frameTimeMs);
    game->loadGame(_savePath);
    checkAfterLoadRng(_trace->header.afterLoadRandomState);
    deterministicComponent->restart(frameTimeMs);
    application()->get<GameKeyboardController>()->reset(); // Reset all pressed buttons.

    if (postLoadCallback)
        postLoadCallback();

    checkState(_trace->header.startState, true);
    application()->get<EngineTraceSimplePlayer>()->playTrace(game, std::move(_trace->events), _tracePath, _flags, std::move(tickCallback));
    checkState(_trace->header.endState, false);
}

void EngineTracePlayer::checkSaveFileSize(int expectedSaveFileSize) {
    int saveFileSize = std::filesystem::file_size(_savePath);
    if (expectedSaveFileSize != saveFileSize) {
        throw Exception("Trace '{}' expected a savegame of size {} bytes, but the size of '{}' is {} bytes",
                        _tracePath, expectedSaveFileSize, _savePath, saveFileSize);
    }
}

void EngineTracePlayer::checkAfterLoadRng(int expectedRandomState) {
    if (_flags & TRACE_PLAYBACK_SKIP_RANDOM_CHECKS)
        return;

    int randomState = grng->peek(1024);
    if (randomState != expectedRandomState) {
        throw Exception("Random state desynchronized after loading a save for trace '{}': expected {}, got {}",
                        _tracePath, expectedRandomState, randomState);
    }
}

void EngineTracePlayer::checkState(const EventTraceGameState &expectedState, bool isStart) {
    if (_flags & TRACE_PLAYBACK_SKIP_STATE_CHECKS)
        return;

    std::string_view where = isStart ? "start" : "end";

    EventTraceGameState state = EngineTraceStateAccessor::makeGameState();
    if (state.locationName != expectedState.locationName) {
        throw Exception("Unexpected location name at the {} of trace '{}': expected '{}', got '{}'",
                        where, _tracePath, expectedState.locationName, state.locationName);
    }
    if (state.partyPosition != expectedState.partyPosition) {
        throw Exception("Unexpected party position at the {} of trace '{}': expected ({}, {}, {}), got ({}, {}, {})",
                        where, _tracePath,
                        expectedState.partyPosition.x, expectedState.partyPosition.y, expectedState.partyPosition.z,
                        state.partyPosition.x, state.partyPosition.y, state.partyPosition.z);
    }
}
