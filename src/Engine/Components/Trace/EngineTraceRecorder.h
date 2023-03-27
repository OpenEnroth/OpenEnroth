#pragma once

#include <string>

#include "Library/Application/PlatformApplicationAware.h"

class EngineTraceComponent;
class EngineDeterministicComponent;
class EngineControlComponent;
class GameKeyboardController;

/**
 * Component that exposes a trace recording interface. Doesn't have a `Component` in its name because who likes
 * class names half a screen long.
 *
 * Depends on `EngineControlComponent`, `EngineDeterministicComponent` and `EngineTraceComponent`, install them into
 * `PlatformApplication` first.
 *
 * Note that the difference from `EngineTraceComponent` is that this component isn't dumb and offers a complete solution
 * to trace recording. Traces that were recorded with `startTraceRecording` / `finishTraceRecording` can then be played
 * back with `EngineTracePlayer`.
 *
 * Some notes on how this works. When starting trace recording:
 * - The game is saved.
 * - Then it is loaded right away. Loading is managed from the control thread.
 * - Then deterministic mode is entered, and actual event recording is started.
 *
 * Why do we need to start trace recording with loading the game? Loading the game updates the states of all actors
 * on the map, and since trace recording and playback must start in the same state, the only option that we have is to
 * start both trace playback and trace recording by loading the same save.
 */
class EngineTraceRecorder : private PlatformApplicationAware {
 public:
    ~EngineTraceRecorder();

    /**
     * Starts trace recording.
     *
     * @param savePath                  Path to save file.
     * @param tracePath                 Path to trace file.
     */
    void startRecording(const std::string &savePath, const std::string &tracePath);
    void finishRecording();

    [[nodiscard]] bool isRecording() const {
        return _isRecording;
    }

 private:
    friend class PlatformIntrospection;

    virtual void installNotify() override;
    virtual void removeNotify() override;

 private:
    bool _isRecording = false;
    std::string _saveFilePath;
    std::string _traceFilePath;
    int _oldFpsLimit = 0;
    EngineControlComponent *_controlComponent = nullptr;
    EngineDeterministicComponent *_deterministicComponent = nullptr;
    EngineTraceComponent *_traceComponent = nullptr;
    GameKeyboardController *_keyboardController = nullptr;
};
