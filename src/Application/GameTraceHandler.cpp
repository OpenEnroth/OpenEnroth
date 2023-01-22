#include "GameTraceHandler.h"

#include <cassert>

#include "Engine/IocContainer.h"

#include "Library/Logger/Logger.h"
#include "Library/Trace/EventTracer.h"

GameTraceHandler::GameTraceHandler(EventTracer *tracer) : PlatformEventFilter({PlatformEvent::KeyPress, PlatformEvent::KeyRelease}), _tracer(tracer) {
    assert(tracer);
}

bool GameTraceHandler::KeyPressEvent(const PlatformKeyEvent *event) {
    if (isTriggerKey(event) && _waitingForKeyRelease)
        return true; // Ignore auto-repeats

    if (isTriggerKeySequence(event)) {
        _waitingForKeyRelease = true;

        if (!_tracer->isTracing()) {
            // TODO(captainurist) : also save game here
            _tracer->start();
            logger->Info("Tracing started.");
        } else {
            _tracer->finish("trace.json");
            logger->Info("Tracing finished.");
        }
        return true;
    }

    return false;
}

bool GameTraceHandler::KeyReleaseEvent(const PlatformKeyEvent *event) {
    if (isTriggerKey(event) && _waitingForKeyRelease) {
        _waitingForKeyRelease = false;

        // We didn't pass the key press down, so don't pass the key release down either, even if it was released
        // after the mods, and thus isTriggerKeySequence(event) is false at this point.
        return true;
    }

    return false;
}

bool GameTraceHandler::isTriggerKey(const PlatformKeyEvent *event) const {
    // TODO(captainurist) : make configurable
    return event->key == PlatformKey::R;
}

bool GameTraceHandler::isTriggerKeySequence(const PlatformKeyEvent *event) const {
    // TODO(captainurist) : make configurable
    return event->key == PlatformKey::R && event->mods == (PlatformModifier::Ctrl | PlatformModifier::Shift);
}
