#pragma once

#include "Library/Platform/Filters/PlatformEventFilter.h"
#include "Library/Platform/Application/PlatformApplicationAware.h"

// TODO(captainurist): tbh we just need a hotkey system instead of this monstrosity.
/**
 * Event filter that reacts to start/stop recording hotkey and calls into event tracer to start/stop recording.
 *
 * Note that this event filter should be installed last (or at least after the `EventTracer`), so that the start/stop
 * keystrokes don't end up being recorded. Technically there's nothing wrong with recording them, but there's little
 * point in doing so.
 */
class GameTraceHandler : private PlatformEventFilter, private PlatformApplicationAware {
 public:
    GameTraceHandler();
    virtual ~GameTraceHandler() = default;

    virtual bool keyPressEvent(const PlatformKeyEvent *event) override;
    virtual bool keyReleaseEvent(const PlatformKeyEvent *event) override;

 private:
    friend class PlatformIntrospection;

    bool isTriggerKey(const PlatformKeyEvent *event) const;
    bool isTriggerKeySequence(const PlatformKeyEvent *event) const;

 private:
    bool _waitingForKeyRelease = false;
};
