#include "GameTraceHandler.h"

#include <algorithm>
#include <string>
#include <string_view>

#include "GameConfig.h"

#include "Engine/Resources/EngineFileSystem.h"
#include "Engine/Components/Control/EngineControlComponent.h"
#include "Engine/Components/Trace/EngineTraceRecorder.h"

#include "Library/Platform/Application/PlatformApplication.h"
#include "Library/Logger/Logger.h"
#include "Library/Serialization/Serialization.h"
#include "Utility/String/Format.h"

GameTraceHandler::GameTraceHandler(GameConfig *config)
    : PlatformEventFilter({EVENT_KEY_PRESS, EVENT_KEY_RELEASE}), _config(config)
{}

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
                EngineTraceRecording recording = tracer->finishRecording(game);

                // Pick the number one past the highest existing traceNNNNN file, so that the fresh trace always
                // sorts last.
                int maxIndex = 0;
                for (const DirectoryEntry &entry : ufs->ls("")) {
                    int index = 0;
                    if (entry.name.starts_with("trace") && tryDeserialize(std::string_view(entry.name).substr(5, 5), &index))
                        maxIndex = std::max(maxIndex, index);
                }
                std::string tracePath = fmt::format("trace{:05}.json", maxIndex + 1);
                std::string savePath = fmt::format("trace{:05}.mm7", maxIndex + 1);

                ufs->write(tracePath, recording.trace);
                ufs->write(savePath, recording.save);
                MM_INFO("Trace saved to {} and {}", ufs->displayPath(tracePath), ufs->displayPath(savePath));
            } else {
                tracer->startRecording(game);
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
    return event->key == _config->debug.TraceKey.value();
}

bool GameTraceHandler::isTriggerKeySequence(const PlatformKeyEvent *event) const {
    PlatformModifiers mods = MOD_CTRL | MOD_SHIFT;

    return isTriggerKey(event) && (event->mods & mods) == mods;
}
