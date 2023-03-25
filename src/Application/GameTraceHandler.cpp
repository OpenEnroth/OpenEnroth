#include "GameTraceHandler.h"

#include <cassert>

#include "Engine/IocContainer.h"
#include "Engine/Components/Trace/EngineTracer.h"

#include "Library/Logger/Logger.h"

GameTraceHandler::GameTraceHandler(EngineTracer *tracer) : PlatformEventFilter({EVENT_KEY_PRESS, EVENT_KEY_RELEASE}), _tracer(tracer) {
    assert(tracer);
}

bool GameTraceHandler::keyPressEvent(const PlatformKeyEvent *event) {
    if (isTriggerKey(event) && _waitingForKeyRelease) {
        if (event->isAutoRepeat) {
            return true; // Ignore auto-repeats
        } else {
            _waitingForKeyRelease = false; // Looks like we've missed the key release.
        }
    }

    if (isTriggerKeySequence(event)) {
        _waitingForKeyRelease = true;

        if (_tracer->state() == EngineTracer::CHILLING) {
            _tracer->startTraceRecording("trace.mm7", "trace.json");
        } else {
            assert(_tracer->state() == EngineTracer::RECORDING);
            _tracer->finishTraceRecording();
        }
        return true;
    }

    return false;
}

bool GameTraceHandler::keyReleaseEvent(const PlatformKeyEvent *event) {
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
    PlatformModifiers mods = MOD_CTRL | MOD_SHIFT;

    return event->key == PlatformKey::R && (event->mods & mods) == mods;
}
