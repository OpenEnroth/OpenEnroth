#pragma once

#include <string>
#include <vector>
#include <memory>

#include "Library/Platform/Application/PlatformApplicationAware.h"

#include "EngineTraceEnums.h"

class EngineController;
class PaintEvent;
class PlatformEvent;

/**
 * Component that can be used to play recorded events.
 *
 * Note that this component is intentionally very dumb. Calling `playTrace` just plays all the passed events in
 * sequence.
 *
 * @see EngineTracePlayer
 */
class EngineTraceSimplePlayer : private PlatformApplicationAware {
 public:
    EngineTraceSimplePlayer();
    ~EngineTraceSimplePlayer();

    /**
     * @param game                      Engine controller.
     * @param events                    Vector of events to play. Passed vector is consumed by this function.
     * @param traceDisplayPath          Path to trace file that the events were loaded from. Used only for error
     *                                  reporting.
     * @param flags                     Playback flags.
     */
    void playTrace(EngineController *game, std::vector<std::unique_ptr<PlatformEvent>> events,
                   std::string_view traceDisplayPath, EngineTracePlaybackFlags flags);

    bool isPlaying() const {
        return _playing;
    }

 private:
    friend class PlatformIntrospection; // Give access to private bases.

    void checkTime(const PaintEvent *paintEvent);
    void checkRng(const PaintEvent *paintEvent);

 private:
    bool _playing = false;
    std::string _traceDisplayPath;
    EngineTracePlaybackFlags _flags;
};
