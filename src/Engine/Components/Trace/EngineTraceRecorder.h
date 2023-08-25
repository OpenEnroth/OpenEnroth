#pragma once

#include <string>
#include <memory>
#include <vector>
#include "Library/Application/PlatformApplicationAware.h"
#include "EngineTraceEnums.h"

class EngineController;
class EngineTraceSimpleRecorder;
class EngineDeterministicComponent;
class GameKeyboardController;
class EventTrace;

/**
 * Component that exposes a trace recording interface. Doesn't have a `Component` in its name because who likes
 * class names half a screen long.
 *
 * Depends on `EngineDeterministicComponent` and `EngineTraceSimpleRecorder`, install them into `PlatformApplication`
 * first.
 *
 * Note that the difference from `EngineTraceSimpleRecorder` is that this component isn't dumb and offers a
 * complete solution to trace recording. Traces that were recorded with `startRecording` / `finishRecording` can
 * then be played back with `EngineTracePlayer`.
 *
 * Some notes on how this works. When starting trace recording:
 * - The game is saved.
 * - Then it is loaded right away. Loading is managed from the control thread.
 * - Then deterministic mode is entered, and actual event recording is started.
 *
 * Why do we need to start trace recording with loading the game? Loading the game updates the states of all actors
 * on the map, and since trace recording and playback must start in the same state, the only option that we have is to
 * start both trace playback and trace recording by loading the same save.
 *
 * @see EngineTracePlayer
 */
class EngineTraceRecorder : private PlatformApplicationAware {
 public:
    EngineTraceRecorder();
    ~EngineTraceRecorder();

    /**
     * Starts trace recording.
     *
     * Note that this method needs to be called from the control thread, see `EngineControlComponent`.
     *
     * @param game                      Engine controller.
     * @param savePath                  Path to save file.
     * @param tracePath                 Path to trace file.
     * @param flags                     Recording flags.
     */
    void startRecording(EngineController *game, const std::string &savePath, const std::string &tracePath, EngineTraceRecordingFlags flags = 0);

    /**
     * Finishes trace recording & saves the trace file.
     *
     * Note that this method needs to be called from the control thread, see `EngineControlComponent`.
     *
     * @param game                      Engine controller.
     */
    void finishRecording(EngineController *game);

    /**
     * @return                          Whether recording is in progress. Make sure to call this method only from the
     *                                  control thread, otherwise you basically can't reason about the current recording
     *                                  state.
     */
    [[nodiscard]] bool isRecording() const {
        return _trace != nullptr;
    }

 private:
    friend class PlatformIntrospection;

    virtual void installNotify() override;
    virtual void removeNotify() override;

 private:
    std::string _savePath;
    std::string _tracePath;
    int _oldFpsLimit = 0;
    std::unique_ptr<EventTrace> _trace;
    EngineDeterministicComponent *_deterministicComponent = nullptr;
    EngineTraceSimpleRecorder *_simpleRecorder = nullptr;
    GameKeyboardController *_keyboardController = nullptr;
};
