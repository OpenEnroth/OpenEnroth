#include "EngineTraceRecorder.h"

#include <cassert>
#include <memory>
#include <utility>

#include "Application/GameKeyboardController.h" // TODO(captainurist): Engine -> Application dependency

#include "Engine/Engine.h"
#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Deterministic/EngineDeterministicComponent.h"
#include "Engine/Random/Random.h"

#include "Library/Platform/Application/PlatformApplication.h"
#include "Library/FileSystem/Memory/MemoryFileSystem.h"
#include "Library/Trace/EventTrace.h"
#include "Library/Logger/Logger.h"

#include "Utility/ScopeGuard.h"

#include "EngineTraceSimpleRecorder.h"
#include "EngineTraceStateAccessor.h"
#include "Engine/EngineFileSystem.h"

EngineTraceRecorder::EngineTraceRecorder() {}

EngineTraceRecorder::~EngineTraceRecorder() {
    assert(!application()); // We're uninstalled.
}

void EngineTraceRecorder::startRecording(EngineController *game, const Blob &savedGame) {
    assert(!isRecording());

    _savedGame = Blob::share(savedGame);
    _trace = std::make_unique<EventTrace>();
    _configSnapshot = std::make_unique<ConfigPatch>(ConfigPatch::fromConfig(engine->config.get()));

    game->resizeWindow(640, 480);
    game->tick();

    int frameTimeMs = engine->config->debug.TraceFrameTimeMs.value();
    RandomEngineType rngType = engine->config->debug.TraceRandomEngine.value();

    if (!_savedGame)
        _savedGame = game->saveGame();
    _trace->header.saveFileSize = _savedGame.size();

    // Reset all pressed buttons. It's important to do this before loading the game b/c game loading peeks into pressed
    // buttons and does all kinds of weird stuff.
    component<GameKeyboardController>()->reset();

    engine->config->graphics.FPSLimit.setValue(0);
    game->goToMainMenu(); // This might call into a random engine.
    component<EngineDeterministicComponent>()->restart(frameTimeMs, rngType);
    game->loadGame(_savedGame);
    _trace->header.afterLoadRandomState = grng->peek(1024 * 1024);
    component<EngineDeterministicComponent>()->restart(frameTimeMs, rngType);

    _trace->header.startState = EngineTraceStateAccessor::makeGameState();
    EngineTraceStateAccessor::prepareForRecording(engine->config.get(), &_trace->header.config);

    // Same as in EngineTraceRecorder - replace user fs with a filesystem that only has the current save.
    _ramFs = std::make_unique<MemoryFileSystem>("ramfs");
    _ramFs->write("saves/!!!save.mm7", _savedGame);
    _fsRollback.emplace(&ufs, _ramFs.get());

    component<EngineTraceSimpleRecorder>()->startRecording();

    logger->info("Tracing started.");
}

EngineTraceRecording EngineTraceRecorder::finishRecording(EngineController *game) {
    assert(isRecording());

    MM_AT_SCOPE_EXIT({
        _fsRollback.reset(); // Roll it back.
        _ramFs.reset();
        _savedGame = {};
        _trace.reset();
        component<EngineDeterministicComponent>()->finish();
        _configSnapshot->apply(engine->config.get()); // Roll back all config changes.
        _configSnapshot.reset();
    });

    _trace->events = component<EngineTraceSimpleRecorder>()->finishRecording();
    _trace->header.endState = EngineTraceStateAccessor::makeGameState();

    EngineTraceRecording result;
    result.save = std::move(_savedGame);
    result.trace = EventTrace::toJsonBlob(*_trace);
    return result;
}
