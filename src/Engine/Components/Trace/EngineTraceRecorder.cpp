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

#include "EngineTraceComponent.h"

EngineTraceRecorder::EngineTraceRecorder() {}

EngineTraceRecorder::~EngineTraceRecorder() {
    assert(!application()); // We're uninstalled.
}

void EngineTraceRecorder::startRecording(EngineController *game, const std::string &savePath, const std::string &tracePath) {
    assert(!savePath.empty() && !tracePath.empty());
    assert(!isRecording());

    _saveFilePath = savePath;
    _traceFilePath = tracePath;

    game->resizeWindow(640, 480);
    game->tick();

    int frameTimeMs = engine->config->debug.TraceFrameTimeMs.value();
    int traceFpsLimit = 1000 / frameTimeMs;

    _oldFpsLimit = engine->config->graphics.FPSLimit.value();
    engine->config->graphics.FPSLimit.setValue(0); // Load game real quick!
    game->saveGame(savePath);
    _deterministicComponent->startDeterministicSegment(frameTimeMs);
    game->loadGame(savePath);
    _deterministicComponent->startDeterministicSegment(frameTimeMs);
    _keyboardController->reset(); // Reset all pressed buttons.

    _traceComponent->startRecording();
    engine->config->graphics.FPSLimit.setValue(traceFpsLimit);

    logger->info("Tracing started.");
}

void EngineTraceRecorder::finishRecording(EngineController *game) {
    assert(isRecording());

    _deterministicComponent->finish();
    engine->config->graphics.FPSLimit.setValue(_oldFpsLimit);

    EventTrace trace = _traceComponent->finishRecording();
    trace.header.saveFileSize = std::filesystem::file_size(_saveFilePath);
    EventTrace::saveToFile(_traceFilePath, trace);

    logger->info("Trace saved to {} and {}",
                 absolute(std::filesystem::path(_saveFilePath)).generic_string(),
                 absolute(std::filesystem::path(_traceFilePath)).generic_string());

    _traceFilePath.clear();
    _saveFilePath.clear();
}

void EngineTraceRecorder::installNotify() {
    _traceComponent = application()->get<EngineTraceComponent>();
    _deterministicComponent = application()->get<EngineDeterministicComponent>();
    _keyboardController = application()->get<GameKeyboardController>();
}

void EngineTraceRecorder::removeNotify() {
    _traceComponent = nullptr;
    _deterministicComponent = nullptr;
    _keyboardController = nullptr;
}


