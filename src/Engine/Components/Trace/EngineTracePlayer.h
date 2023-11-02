#pragma once

#include <string>
#include <functional>
#include <memory>

#include "Library/Platform/Application/PlatformApplicationAware.h"

#include "EngineTraceEnums.h"

class EngineController;
class EventTrace;
class EventTraceGameState;

/**
 * Component that exposes a trace playback interface.
 *
 * Depends on `EngineDeterministicComponent` and `EngineTraceSimplePlayer`, install them into `PlatformApplication`
 * first.
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
     * @param postLoadCallback          Callback to call once the saved game is loaded.
     * @param tickCallback              Callback to call every frame and also before & after running the trace.
     */
    void playTrace(EngineController *game, const std::string &savePath, const std::string &tracePath,
                   EngineTracePlaybackFlags flags = 0, std::function<void()> postLoadCallback = {},
                   std::function<void()> tickCallback = {});

    [[nodiscard]] bool isPlaying() const {
        return _trace != nullptr;
    }

 private:
    friend class PlatformIntrospection;

    void checkSaveFileSize(int expectedSaveFileSize);
    void checkAfterLoadRng(int expectedRandomState);
    void checkState(const EventTraceGameState &expectedState, bool isStart);

 private:
    std::string _tracePath;
    std::string _savePath;
    EngineTracePlaybackFlags _flags;
    std::unique_ptr<EventTrace> _trace;
};
