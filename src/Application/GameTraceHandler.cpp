#include "GameTraceHandler.h"

#include "Engine/Components/Control/EngineControlComponent.h"
#include "Engine/Components/Trace/EngineTraceRecorder.h"

#include "Library/Platform/Application/PlatformApplication.h"
#include "Library/Logger/Logger.h"

GameTraceHandler::GameTraceHandler() : PlatformEventFilter({EVENT_KEY_PRESS, EVENT_KEY_RELEASE}) {}

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

        component<EngineControlComponent>()->runControlRoutine([this] (EngineController *game) {
            EngineTraceRecorder *tracer = component<EngineTraceRecorder>();
            if (tracer->isRecording()) {
                tracer->finishRecording(game);
            } else {
                tracer->startRecording(game, "trace.mm7", "trace.json");
            }
        });
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
    return event->key == PlatformKey::KEY_R;
}

bool GameTraceHandler::isTriggerKeySequence(const PlatformKeyEvent *event) const {
    // TODO(captainurist) : make configurable
    PlatformModifiers mods = MOD_CTRL | MOD_SHIFT;

    return event->key == PlatformKey::KEY_R && (event->mods & mods) == mods;
}
