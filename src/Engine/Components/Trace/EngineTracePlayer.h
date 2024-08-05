#pragma once

#include <string>
#include <functional>
#include <memory>

#include "Library/Platform/Application/PlatformApplicationAware.h"

#include "EngineTraceEnums.h"
#include "EngineTraceRecording.h"

class EngineController;
struct EventTrace;
struct EventTraceGameState;

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
     * @param recording                 Recorded trace.
     * @param flags                     Playback flags.
     * @param postLoadCallback          Callback to call once the saved game is loaded.
     */
    void playTrace(EngineController *game, const EngineTraceRecording &recording,
                   EngineTracePlaybackFlags flags = 0, std::function<void()> postLoadCallback = {});

    [[nodiscard]] bool isPlaying() const {
        return _trace != nullptr;
    }

 private:
    friend class PlatformIntrospection;

    void checkSaveFileSize(const EngineTraceRecording &recording, int expectedSaveFileSize);
    void checkAfterLoadRng(const EngineTraceRecording &recording, int expectedRandomState);
    void checkState(const EngineTraceRecording &recording, const EventTraceGameState &expectedState, bool isStart);

 private:
    EngineTracePlaybackFlags _flags;
    std::unique_ptr<EventTrace> _trace;
};
