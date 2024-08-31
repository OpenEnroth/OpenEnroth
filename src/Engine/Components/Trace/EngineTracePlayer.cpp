#include "EngineTracePlayer.h"

#include <cassert>
#include <utility>
#include <memory>

#include "Application/GameKeyboardController.h" // TODO(captainurist): Engine -> Application dependency

#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Deterministic/EngineDeterministicComponent.h"
#include "Engine/Random/Random.h"
#include "Engine/Engine.h"
#include "Engine/EngineFileSystem.h"

#include "Library/Trace/PaintEvent.h"
#include "Library/Trace/EventTrace.h"
#include "Library/Platform/Application/PlatformApplication.h"
#include "Library/FileSystem/Memory/MemoryFileSystem.h"

#include "Utility/ScopeGuard.h"
#include "Utility/ScopedRollback.h"
#include "Utility/Exception.h"

#include "EngineTraceStateAccessor.h"
#include "EngineTraceSimplePlayer.h"

EngineTracePlayer::EngineTracePlayer() {}

EngineTracePlayer::~EngineTracePlayer() {
    assert(!application()); // We're uninstalled.
}

void EngineTracePlayer::playTrace(EngineController *game, const EngineTraceRecording &recording,
                                  EngineTracePlaybackFlags flags, std::function<void()> postLoadCallback) {
    assert(!isPlaying());

    _flags = flags;
    _trace = std::make_unique<EventTrace>(EventTrace::fromJsonBlob(recording.trace, application()->window()));

    MM_AT_SCOPE_EXIT({
        _flags = 0;
        _trace.reset();
        component<EngineDeterministicComponent>()->finish();
    });

    checkSaveFileSize(recording, _trace->header.saveFileSize);

    game->resizeWindow(640, 480);
    game->tick();

    EngineTraceStateAccessor::prepareForPlayback(engine->config.get(), _trace->header.config);
    int frameTimeMs = engine->config->debug.TraceFrameTimeMs.value();
    RandomEngineType rngType = engine->config->debug.TraceRandomEngine.value();

    game->goToMainMenu(); // This might call into a random engine.
    component<EngineDeterministicComponent>()->restart(frameTimeMs, rngType);
    game->loadGame(recording.save);
    checkAfterLoadRng(recording, _trace->header.afterLoadRandomState);
    component<EngineDeterministicComponent>()->restart(frameTimeMs, rngType);
    component<GameKeyboardController>()->reset(); // Reset all pressed buttons.

    if (postLoadCallback)
        postLoadCallback();

    // Place the save game in /saves while the trace is playing - we might want to load the save again from
    // inside the trace.
    MemoryFileSystem ramFs("ramfs");
    ramFs.write("saves/!!!save.mm7", recording.save);
    ScopedRollback<FileSystem *> rollback(&ufs, &ramFs);

    checkState(recording, _trace->header.startState, true);
    component<EngineTraceSimplePlayer>()->playTrace(game, std::move(_trace->events), recording.trace.displayPath(), _flags);
    checkState(recording, _trace->header.endState, false);
}

void EngineTracePlayer::checkSaveFileSize(const EngineTraceRecording &recording, int expectedSaveFileSize) {
    if (expectedSaveFileSize != recording.save.size()) {
        throw Exception("Trace '{}' expected a savegame of size {} bytes, but the size of '{}' is {} bytes",
                        recording.trace.displayPath(), expectedSaveFileSize, recording.save.displayPath(), recording.save.size());
    }
}

void EngineTracePlayer::checkAfterLoadRng(const EngineTraceRecording &recording, int expectedRandomState) {
    if (_flags & TRACE_PLAYBACK_SKIP_RANDOM_CHECKS)
        return;

    int randomState = grng->peek(1024 * 1024);
    if (randomState != expectedRandomState) {
        throw Exception("Random state desynchronized after loading a save for trace '{}': expected {}, got {}",
                        recording.trace.displayPath(), expectedRandomState, randomState);
    }
}

void EngineTracePlayer::checkState(const EngineTraceRecording &recording, const EventTraceGameState &expectedState, bool isStart) {
    if (_flags & TRACE_PLAYBACK_SKIP_STATE_CHECKS)
        return;

    std::string_view where = isStart ? "start" : "end";

    EventTraceGameState state = EngineTraceStateAccessor::makeGameState();
    if (state.locationName != expectedState.locationName) {
        throw Exception("Unexpected location name at the {} of trace '{}': expected '{}', got '{}'",
                        where, recording.trace.displayPath(), expectedState.locationName, state.locationName);
    }
    if (state.partyPosition != expectedState.partyPosition) {
        throw Exception("Unexpected party position at the {} of trace '{}': expected ({}, {}, {}), got ({}, {}, {})",
                        where, recording.trace.displayPath(),
                        expectedState.partyPosition.x, expectedState.partyPosition.y, expectedState.partyPosition.z,
                        state.partyPosition.x, state.partyPosition.y, state.partyPosition.z);
    }
    if (state.characters.size() != expectedState.characters.size()) {
        throw Exception("Unexpected number of characters at the {} of trace '{}': expected {}, got {}",
                        where, recording.trace.displayPath(), expectedState.characters.size(), state.characters.size());
    }
    for (size_t i = 0; i < state.characters.size(); i++) {
        static constexpr std::initializer_list<std::pair<int EventTraceCharacterState::*, const char *>> attributes = {
            {&EventTraceCharacterState::hp, "hp"},
            {&EventTraceCharacterState::mp, "mp"},
            {&EventTraceCharacterState::might, "might"},
            {&EventTraceCharacterState::intelligence, "intelligence"},
            {&EventTraceCharacterState::personality, "personality"},
            {&EventTraceCharacterState::endurance, "endurance"},
            {&EventTraceCharacterState::accuracy, "accuracy"},
            {&EventTraceCharacterState::speed, "speed"},
            {&EventTraceCharacterState::luck, "luck"},
        };

        for (const auto &[attribute, attributeName] : attributes) {
            int expected = expectedState.characters[i].*attribute;
            int actual = state.characters[i].*attribute;
            if (expected != actual) {
                throw Exception("Unexpected {} of character #{} at the {} of trace '{}': expected {}, got {}",
                                attributeName, i, where, recording.trace.displayPath(), expected, actual);
            }
        }
    }
}
