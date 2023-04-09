#pragma once

#include <string>
#include <functional>
#include <memory>

#include "Library/Application/PlatformApplicationAware.h"

#include "Utility/Flags.h"

#include "EngineTracePlaybackFlags.h"

class EngineController;
class EngineDeterministicComponent;
class GameKeyboardController;
class EventTrace;

/**
 * Component that exposes a trace playback interface.
 *
 * Depends on `EngineDeterministicComponent`, install it into `PlatformApplication` first.
 *
 * @see EngineTraceRecorder
 */
class EngineTracePlayer : private PlatformApplicationAware {
 public:
    EngineTracePlayer();
    ~EngineTracePlayer();

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

    [[nodiscard]] bool isPlaying() const {
        return _trace != nullptr;
    }

    void prepareTrace(EngineController *game, const std::string &savePath, const std::string &tracePath);
    void playPreparedTrace(EngineController *game, EngineTracePlaybackFlags flags = 0);

 private:
    friend class PlatformIntrospection;

    virtual void installNotify() override;
    virtual void removeNotify() override;

 private:
    std::string _tracePath;
    std::unique_ptr<EventTrace> _trace;
    EngineDeterministicComponent *_deterministicComponent = nullptr;
    GameKeyboardController *_keyboardController = nullptr;
};
