#pragma once

#include <memory>
#include <string>
#include <functional>

#include "Library/Application/PlatformApplicationAware.h"

#include "Utility/Flags.h"

class EngineController;
class EngineTracePlugin;
class EngineDeterministicPlugin;
class EngineControlPlugin;
class GameKeyboardController;

// TODO(captainurist): It would make more sense to split this class in two. And move both into a separate
// folder in Engine.
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
     * Plays a previously recorded trace. Can be called only from a control thread.
     *
     * @param game                      Engine controller.
     * @param savePath                  Path to save file.
     * @param tracePath                 Path to trace file.
     * @param postLoadCallback          Callback to call once the savegame is loaded.
     */
    void playTrace(EngineController *game, const std::string &savePath, const std::string &tracePath, std::function<void()> postLoadCallback);

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
    EngineControlPlugin *_controlPlugin = nullptr;
    EngineDeterministicPlugin *_deterministicPlugin = nullptr;
    EngineTracePlugin *_tracePlugin = nullptr;
    GameKeyboardController *_keyboardController = nullptr;
};

MM_DECLARE_OPERATORS_FOR_FLAGS(EngineTracer::Options)
