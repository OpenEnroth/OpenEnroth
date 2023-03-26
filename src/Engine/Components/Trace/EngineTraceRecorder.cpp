#include "EngineTraceRecorder.h"

#include <cassert>
#include <filesystem>

#include "Application/GameKeyboardController.h" // TODO(captainurist): Engine -> Application dependency

#include "Engine/Engine.h"
#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Control/EngineControlComponent.h"
#include "Engine/Components/Deterministic/EngineDeterministicComponent.h"

#include "Library/Application/PlatformApplication.h"

#include "EngineTraceComponent.h"

EngineTraceRecorder::~EngineTraceRecorder() {
    assert(!application()); // We're uninstalled.
}

void EngineTraceRecorder::startRecording(const std::string &savePath, const std::string &tracePath) {
    assert(!isRecording());

    _isRecording = true;
    _saveFilePath = savePath;
    _traceFilePath = tracePath;

    // TODO(captainurist): this is actually unsafe, we need to cancel running routines in dtor if there are any.
    _controlComponent->runControlRoutine([this, savePath] (EngineController *game) {
        game->resizeWindow(640, 480);
        game->tick();

        _oldFpsLimit = engine->config->graphics.FPSLimit.Get();
        engine->config->graphics.FPSLimit.Set(1000); // Load game real quick!
        game->saveGame(savePath);
        _deterministicComponent->enterDeterministicMode();
        game->loadGame(savePath);
        _deterministicComponent->resetDeterministicState();
        _keyboardController->reset(); // Reset all pressed buttons.

        _traceComponent->start();
        engine->config->graphics.FPSLimit.Set(EngineDeterministicComponent::TARGET_FPS); // But don't turn the party into a wall-running doomguy.

        logger->Info("Tracing started.");
    });
}

void EngineTraceRecorder::finishRecording() {
    assert(isRecording());

    // The code below needs to go into a control routine because it should be run AFTER the previous control routine
    // is finished, and it could take a while for it to finish.
    _controlComponent->runControlRoutine([this, tracePath = _traceFilePath, savePath = _saveFilePath] (EngineController *game) {
        _deterministicComponent->leaveDeterministicMode();
        engine->config->graphics.FPSLimit.Set(_oldFpsLimit);

        EventTrace trace = _traceComponent->finish();
        trace.header.saveFileSize = std::filesystem::file_size(savePath); // This function can throw.
        EventTrace::saveToFile(tracePath, trace);

        logger->Info("Trace saved to {} and {}",
                     absolute(std::filesystem::path(tracePath)).generic_string(),
                     absolute(std::filesystem::path(savePath)).generic_string());
    });

    _isRecording = false;
    _traceFilePath.clear();
    _saveFilePath.clear();
}

void EngineTraceRecorder::installNotify() {
    _controlComponent = application()->get<EngineControlComponent>();
    _traceComponent = application()->get<EngineTraceComponent>();
    _deterministicComponent = application()->get<EngineDeterministicComponent>();
    _keyboardController = application()->get<GameKeyboardController>();
}

void EngineTraceRecorder::removeNotify() {
    _controlComponent = nullptr;
    _traceComponent = nullptr;
    _deterministicComponent = nullptr;
    _keyboardController = nullptr;
}


