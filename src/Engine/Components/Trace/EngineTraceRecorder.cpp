#include "EngineTraceRecorder.h"

#include <cassert>
#include <filesystem>
#include <utility>
#include <memory>

#include "Application/GameKeyboardController.h" // TODO(captainurist): Engine -> Application dependency

#include "Engine/Engine.h"
#include "Engine/EngineIocContainer.h"
#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Deterministic/EngineDeterministicComponent.h"

#include "Library/Application/PlatformApplication.h"
#include "Library/Trace/EventTrace.h"
#include "Library/Logger/Logger.h"
#include "Library/Random/Random.h"

#include "Utility/ScopeGuard.h"

#include "EngineTraceSimpleRecorder.h"
#include "EngineTraceStateAccessor.h"

EngineTraceRecorder::EngineTraceRecorder() {}

EngineTraceRecorder::~EngineTraceRecorder() {
    assert(!application()); // We're uninstalled.
}

void EngineTraceRecorder::startRecording(EngineController *game, const std::string &savePath, const std::string &tracePath, EngineTraceRecordingFlags flags) {
    assert(!savePath.empty() && !tracePath.empty());
    assert(!isRecording());

    _savePath = savePath;
    _tracePath = tracePath;
    _trace = std::make_unique<EventTrace>();

    game->resizeWindow(640, 480);
    engine->config->window.MouseGrab.setValue(false); // TODO(captainurist): move all of this, including resetForTest(), into a single place.
    engine->config->debug.NoVideo.setValue(true);
    game->tick();

    int frameTimeMs = engine->config->debug.TraceFrameTimeMs.value();
    RandomEngineType rngType = engine->config->debug.TraceRandomEngine.value();
    int traceFpsLimit = 1000 / frameTimeMs;

    _trace->header.config = EngineTraceStateAccessor::makeConfigPatch(engine->config.get());
    _oldFpsLimit = engine->config->graphics.FPSLimit.value();
    engine->config->graphics.FPSLimit.setValue(0); // Load game real quick!

    if (!(flags & TRACE_RECORDING_LOAD_EXISTING_SAVE))
        game->saveGame(savePath);
    _trace->header.saveFileSize = std::filesystem::file_size(_savePath);

    game->goToMainMenu(); // This might call into a random engine.
    _deterministicComponent->restart(frameTimeMs, rngType);
    game->loadGame(savePath);
    _trace->header.afterLoadRandomState = grng->peek(1024);
    _deterministicComponent->restart(frameTimeMs, rngType);
    _keyboardController->reset(); // Reset all pressed buttons.

    _trace->header.startState = EngineTraceStateAccessor::makeGameState();
    _simpleRecorder->startRecording();

    engine->config->graphics.FPSLimit.setValue(traceFpsLimit);
    logger->info("Tracing started.");
}

void EngineTraceRecorder::finishRecording(EngineController *game) {
    assert(isRecording());

    MM_AT_SCOPE_EXIT({
        _tracePath.clear();
        _savePath.clear();
        _trace.reset();
        _deterministicComponent->finish();
    });

    engine->config->graphics.FPSLimit.setValue(_oldFpsLimit);

    _trace->events = _simpleRecorder->finishRecording();
    _trace->header.endState = EngineTraceStateAccessor::makeGameState();

    EventTrace::saveToFile(_tracePath, *_trace);

    logger->info("Trace saved to {} and {}",
                 absolute(std::filesystem::path(_savePath)).generic_string(),
                 absolute(std::filesystem::path(_tracePath)).generic_string());
}

void EngineTraceRecorder::installNotify() {
    _simpleRecorder = application()->get<EngineTraceSimpleRecorder>();
    _deterministicComponent = application()->get<EngineDeterministicComponent>();
    _keyboardController = application()->get<GameKeyboardController>();
}

void EngineTraceRecorder::removeNotify() {
    _simpleRecorder = nullptr;
    _deterministicComponent = nullptr;
    _keyboardController = nullptr;
}


