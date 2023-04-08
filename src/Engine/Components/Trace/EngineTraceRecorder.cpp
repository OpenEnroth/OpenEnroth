#include "EngineTraceRecorder.h"

#include <cassert>
#include <filesystem>
#include <utility>
#include <memory>

#include "Application/GameKeyboardController.h" // TODO(captainurist): Engine -> Application dependency

#include "Engine/Engine.h"
#include "Engine/Components/Control/EngineController.h"
#include "Engine/Components/Control/EngineControlComponent.h"
#include "Engine/Components/Deterministic/EngineDeterministicComponent.h"

#include "Library/Application/PlatformApplication.h"
#include "Library/Trace/EventTrace.h"

#include "EngineTraceComponent.h"

EngineTraceRecorder::~EngineTraceRecorder() {
    assert(!application()); // We're uninstalled.
}

void EngineTraceRecorder::startRecording(const std::string &savePath, const std::string &tracePath) {
    assert(!isRecording());

    _isRecording = true;
    _trace = EventTrace();
    _saveFilePath = savePath;
    _traceFilePath = tracePath;

    // TODO(captainurist): this is actually unsafe, we need to cancel running routines in dtor if there are any.
    _controlComponent->runControlRoutine([this, savePath] (EngineController *game) {
        game->resizeWindow(640, 480);
        game->tick();

        _trace.header.config = traceConfig();

        _oldFpsLimit = engine->config->graphics.FPSLimit.value();
        engine->config->graphics.FPSLimit.setValue(1000); // Load game real quick!
        game->saveGame(savePath);
        _deterministicComponent->enterDeterministicMode();
        game->loadGame(savePath);
        _deterministicComponent->resetDeterministicState();
        _keyboardController->reset(); // Reset all pressed buttons.

        _traceComponent->start();
        engine->config->graphics.FPSLimit.setValue(EngineDeterministicComponent::TARGET_FPS); // But don't turn the party into a wall-running doomguy.

        logger->info("Tracing started.");
    });
}

void EngineTraceRecorder::finishRecording() {
    assert(isRecording());

    // TODO(captainurist): change into std::move_only_function and drop make_shared
    // The code below needs to go into a control routine because it should be run AFTER the previous control routine
    // is finished, and it could take a while for it to finish.
    _controlComponent->runControlRoutine(
        [this, tracePath = std::move(_traceFilePath), savePath = std::move(_saveFilePath), trace = std::make_shared<EventTrace>(std::move(_trace))] (EngineController *game) {
            _deterministicComponent->leaveDeterministicMode();
            engine->config->graphics.FPSLimit.setValue(_oldFpsLimit);

            trace->events = _traceComponent->finish();
            trace->header.saveFileSize = std::filesystem::file_size(savePath); // This function can throw.
            EventTrace::saveToFile(tracePath, *trace);

            logger->info("Trace saved to {} and {}",
                         absolute(std::filesystem::path(tracePath)).generic_string(),
                         absolute(std::filesystem::path(savePath)).generic_string());
        }
    );

    _isRecording = false;
    _trace = EventTrace();
    _traceFilePath.clear();
    _saveFilePath.clear();
}

std::vector<EventTraceConfigLine> EngineTraceRecorder::traceConfig() {
    std::vector<EventTraceConfigLine> result;
    for (const ConfigSection *section : engine->config->sections()) {
        if (section == &engine->config->window || section == &engine->config->graphics)
            continue; // Don't save window & graphics settings.

        for (const AnyConfigEntry *entry : section->entries()) {
            if (entry == &engine->config->debug.VerboseLogging)
                continue; // This one is managed externally.

            if (entry->string() != entry->defaultString())
                result.push_back({section->name(), entry->name(), entry->string()});
        }
    }
    return result;
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


