#include "EngineTracer.h"

#include <cassert>
#include <utility>
#include <filesystem>

#include "Utility/ScopeGuard.h"
#include "Utility/Format.h"

#include "Library/Trace/PaintEvent.h"
#include "Library/Application/PlatformApplication.h"
#include "Library/Random/Random.h"

#include "Engine/Engine.h"

#include "Application/GameKeyboardController.h" // TODO(captainurist): Engine -> Application dependency

#include "EngineController.h"
#include "EngineControlPlugin.h"
#include "EngineTracePlugin.h"
#include "EngineDeterministicPlugin.h"

EngineTracer::EngineTracer(Options options): _options(options) {}

EngineTracer::~EngineTracer() {
    assert(!application()); // We're uninstalled.
}

void EngineTracer::startTraceRecording(const std::string &savePath, const std::string &tracePath) {
    assert(_options & ENABLE_RECORDING);
    assert(_state == CHILLING);

    _state = RECORDING;
    _saveFilePath = savePath;
    _traceFilePath = tracePath;

    // TODO(captainurist): this is actually unsafe, we need to cancel running routines in dtor if there are any.
    _controlPlugin->runControlRoutine([this, savePath] (EngineController *game) {
        game->resizeWindow(640, 480);
        game->tick();

        _oldFpsLimit = engine->config->graphics.FPSLimit.Get();
        engine->config->graphics.FPSLimit.Set(1000); // Load game real quick!
        game->saveGame(savePath);
        _deterministicPlugin->enterDeterministicMode();
        game->loadGame(savePath);
        _deterministicPlugin->resetDeterministicState();
        _keyboardController->reset(); // Reset all pressed buttons.

        _tracePlugin->start();
        engine->config->graphics.FPSLimit.Set(EngineDeterministicPlugin::TARGET_FPS); // But don't turn the party into a wall-running doomguy.

        logger->Info("Tracing started.");
    });
}

void EngineTracer::finishTraceRecording() {
    assert(_options & ENABLE_RECORDING);
    assert(_state == RECORDING);

    // The code below needs to go into a control routine because it should be run AFTER the previous control routine
    // is finished, and it could take a while for it to finish.
    _controlPlugin->runControlRoutine([this, tracePath = _traceFilePath, savePath = _saveFilePath] (EngineController *game) {
        assert(_tracePlugin->isTracing()); // Just a sanity check.
        _deterministicPlugin->leaveDeterministicMode();
        engine->config->graphics.FPSLimit.Set(_oldFpsLimit);

        EventTrace trace = _tracePlugin->finish();
        trace.header.saveFileSize = std::filesystem::file_size(savePath); // This function can throw.
        EventTrace::saveToFile(tracePath, trace);

        logger->Info("Trace saved to %s and %s",
                     absolute(std::filesystem::path(tracePath)).generic_string().c_str(),
                     absolute(std::filesystem::path(savePath)).generic_string().c_str());
    });

    _state = CHILLING;
    _traceFilePath.clear();
    _saveFilePath.clear();
}

void EngineTracer::playTrace(EngineController *game, const std::string &savePath, const std::string &tracePath, std::function<void()> postLoadCallback) {
    assert(_options & ENABLE_PLAYBACK);
    assert(_state == CHILLING);

    _state = PLAYING;
    MM_AT_SCOPE_EXIT(_state = CHILLING);

    EventTrace trace = EventTrace::loadFromFile(tracePath, application()->window());

    int saveFileSize = std::filesystem::file_size(savePath);
    if (trace.header.saveFileSize != -1 && trace.header.saveFileSize != saveFileSize) {
        throw std::runtime_error(fmt::format(
            "Trace '{}' expected a savegame of size {} bytes, but the size of '{}' is {} bytes",
            tracePath, trace.header.saveFileSize, savePath, saveFileSize
        ));
    }
    // TODO(captainurist): add an option to skip all checks

    game->resizeWindow(640, 480);
    game->tick();

    _deterministicPlugin->enterDeterministicMode();
    MM_AT_SCOPE_EXIT(_deterministicPlugin->leaveDeterministicMode());

    game->loadGame(savePath);
    _deterministicPlugin->resetDeterministicState();
    _keyboardController->reset(); // Reset all pressed buttons.

    if (postLoadCallback)
        postLoadCallback();

    for (std::unique_ptr<PlatformEvent> &event : trace.events) {
        if (event->type == EVENT_PAINT) {
            game->tick(1);

            const PaintEvent *paintEvent = static_cast<const PaintEvent *>(event.get());

            int64_t tickCount = application()->platform()->tickCount();
            if (tickCount != paintEvent->tickCount) {
                throw std::runtime_error(fmt::format(
                    "Tick count desynchronized when playing back trace '{}': expected {}, got {}",
                    tracePath, paintEvent->tickCount, tickCount
                ));
            }

            int randomState = grng->Random(1024);
            if (randomState != paintEvent->randomState) {
                throw std::runtime_error(fmt::format(
                    "Random state desynchronized when playing back trace '{}' at {}ms: expected {}, got {}",
                    tracePath, tickCount, paintEvent->randomState, randomState
                ));
            }
        } else {
            game->postEvent(std::move(event));
        }
    }
}

void EngineTracer::installNotify() {
    _controlPlugin = application()->get<EngineControlPlugin>();
    _tracePlugin = application()->get<EngineTracePlugin>();
    _deterministicPlugin = application()->get<EngineDeterministicPlugin>();
    _keyboardController = application()->get<GameKeyboardController>();
    assert((_options & ENABLE_RECORDING) ? _controlPlugin && _deterministicPlugin && _tracePlugin && _keyboardController : true);
    assert((_options & ENABLE_PLAYBACK) ? _controlPlugin && _deterministicPlugin && _keyboardController : true);
}

void EngineTracer::removeNotify() {
    _controlPlugin = nullptr;
    _tracePlugin = nullptr;
    _deterministicPlugin = nullptr;
    _keyboardController = nullptr;
}


