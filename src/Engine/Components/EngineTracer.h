#pragma once

#include <memory>
#include <string>
#include <functional>

#include "Library/Application/PlatformApplicationAware.h"

#include "Utility/Flags.h"

#include "EngineTracePlaybackFlags.h"

class EngineController;
class EngineTraceComponent;
class EngineDeterministicComponent;
class EngineControlComponent;
class GameKeyboardController;

// TODO(captainurist): It would make more sense to split this class in two. And move both into a separate
// folder in Engine.
/**
 * Component that exposes a trace playback and trace recording interface.
 *
 * Depends on `EngineControlComponent`, `EngineDeterministicComponent` and (if you use `ENABLE_RECORDING`)
 * `EngineTraceComponent`, install them into `PlatformApplication` first.
 *
 * Note that the difference from `EngineTraceComponent` is that this component isn't dumb and offers a complete solution
 * to trace recording and playback. Traces that were recorded with `startTraceRecording` / `finishTraceRecording` can
 * then be played back via `playTrace`.
 *
 * Some notes on how this works. When starting trace recording:
 * - The game is saved.
 * - Then it is loaded right away. Loading is managed from the control thread.
 * - Then deterministic mode is entered, and actual event recording is started.
 *
 * Why do we need to start trace recording with loading the game? Loading the game updates the states of all actors
 * on the map, and since trace recording and playback must start in same state, the only option that we have is to
 * start both trace playback and trace recording by loading the same save.
 */
class EngineTracer : private PlatformApplicationAware {
 public:
    enum class State {
        CHILLING,
        RECORDING,
        PLAYING
    };
    using enum State;

    enum class Option {
        ENABLE_RECORDING = 0x1,
        ENABLE_PLAYBACK = 0x2
    };
    using enum Option;
    MM_DECLARE_FLAGS(Options, Option)

    explicit EngineTracer(Options options);
    ~EngineTracer();

    State state() const {
        return _state;
    }

    /**
     * Starts trace recording.
     *
     * @param savePath                  Path to save file.
     * @param tracePath                 Path to trace file.
     */
    void startTraceRecording(const std::string &savePath, const std::string &tracePath);
    void finishTraceRecording();

    /**
     * Plays a previously recorded trace. Can be called only from a control thread of `EngineControlComponent`.
     *
     * @param game                      Engine controller.
     * @param savePath                  Path to save file.
     * @param tracePath                 Path to trace file.
     * @param flags                     Playback flags.
     * @param postLoadCallback          Callback to call once the savegame is loaded.
     * @see EngineControlComponent
     */
    void playTrace(EngineController *game, const std::string &savePath, const std::string &tracePath,
                   EngineTracePlaybackFlags flags = 0, std::function<void()> postLoadCallback = {});

 private:
    friend class PlatformIntrospection;

    virtual void installNotify() override;
    virtual void removeNotify() override;

 private:
    Options _options;
    State _state = CHILLING;
    std::string _saveFilePath;
    std::string _traceFilePath;
    int _oldFpsLimit = 0;
    EngineControlComponent *_controlComponent = nullptr;
    EngineDeterministicComponent *_deterministicComponent = nullptr;
    EngineTraceComponent *_traceComponent = nullptr;
    GameKeyboardController *_keyboardController = nullptr;
};

MM_DECLARE_OPERATORS_FOR_FLAGS(EngineTracer::Options)
