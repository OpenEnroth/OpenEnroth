#include "EngineTracePlayer.h"

#include <cassert>
#include <utility>
#include <filesystem>
#include <memory>

#include "Application/GameKeyboardController.h" // TODO(captainurist): Engine -> Application dependency

#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Deterministic/EngineDeterministicComponent.h"
#include "Engine/Random/Random.h"
#include "Engine/Engine.h"

#include "Library/Trace/PaintEvent.h"
#include "Library/Trace/EventTrace.h"
#include "Library/Platform/Application/PlatformApplication.h"

#include "Utility/ScopeGuard.h"
#include "Utility/Exception.h"

#include "EngineTraceStateAccessor.h"
#include "EngineTraceSimplePlayer.h"

EngineTracePlayer::EngineTracePlayer() {}

EngineTracePlayer::~EngineTracePlayer() {
    assert(!application()); // We're uninstalled.
}

void EngineTracePlayer::playTrace(EngineController *game, std::string_view savePath, std::string_view tracePath,
                                  EngineTracePlaybackFlags flags, std::function<void()> postLoadCallback) {
    assert(!isPlaying());

    _tracePath = tracePath;
    _savePath = savePath;
    _flags = flags;
    _trace = std::make_unique<EventTrace>(EventTrace::loadFromFile(_tracePath, application()->window()));

    MM_AT_SCOPE_EXIT({
        _tracePath.clear();
        _savePath.clear();
        _flags = 0;
        _trace.reset();
        component<EngineDeterministicComponent>()->finish();
    });

    checkSaveFileSize(_trace->header.saveFileSize);

    game->resizeWindow(640, 480);
    game->tick();

    EngineTraceStateAccessor::prepareForPlayback(engine->config.get(), _trace->header.config);
    int frameTimeMs = engine->config->debug.TraceFrameTimeMs.value();
    RandomEngineType rngType = engine->config->debug.TraceRandomEngine.value();

    game->goToMainMenu(); // This might call into a random engine.
    component<EngineDeterministicComponent>()->restart(frameTimeMs, rngType);
    game->loadGame(_savePath);
    checkAfterLoadRng(_trace->header.afterLoadRandomState);
    component<EngineDeterministicComponent>()->restart(frameTimeMs, rngType);
    component<GameKeyboardController>()->reset(); // Reset all pressed buttons.

    if (postLoadCallback)
        postLoadCallback();

    checkState(_trace->header.startState, true);
    component<EngineTraceSimplePlayer>()->playTrace(game, std::move(_trace->events), _tracePath, _flags);
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

    int randomState = grng->peek(1024 * 1024);
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
    if (state.characters.size() != expectedState.characters.size()) {
        throw Exception("Unexpected number of characters at the {} of trace '{}': expected {}, got {}",
                        where, _tracePath, expectedState.characters.size(), state.characters.size());
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
                                attributeName, i, where, _tracePath, expected, actual);
            }
        }
    }
}
