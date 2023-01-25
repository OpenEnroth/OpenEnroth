#pragma once

#include "Platform/Filters/PlatformEventFilter.h"

class EventTracer;

/**
 * Event filter that reacts to start/stop recording hotkey and calls into event tracer to start/stop recording.
 *
 * Note that this event filter should be installed last (or at least after the `EventTracer`), so that the start/stop
 * keystrokes don't end up being recorded. Technically there's nothing wrong with recording them, but there's little
 * point in doing so.
 */
class GameTraceHandler : public PlatformEventFilter {
 public:
    explicit GameTraceHandler(EventTracer *tracer);
    virtual ~GameTraceHandler() = default;

    virtual bool KeyPressEvent(const PlatformKeyEvent *event) override;
    virtual bool KeyReleaseEvent(const PlatformKeyEvent *event) override;

 private:
    bool isTriggerKey(const PlatformKeyEvent *event) const;
    bool isTriggerKeySequence(const PlatformKeyEvent *event) const;

 private:
    EventTracer *_tracer = nullptr;
    bool _waitingForKeyRelease = false;
};
