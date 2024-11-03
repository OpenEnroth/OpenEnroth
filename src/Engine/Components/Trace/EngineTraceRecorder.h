#pragma once

#include <memory>
#include <optional>

#include "Library/Platform/Application/PlatformApplicationAware.h"

#include "Utility/Memory/Blob.h"
#include "Utility/ScopedRollback.h"

#include "EngineTraceRecording.h"

class EngineController;
struct EventTrace;
class ConfigPatch;
class FileSystem;

/**
 * Component that exposes a trace recording interface. Doesn't have a `Component` in its name because who likes
 * class names half a screen long.
 *
 * Depends on `EngineDeterministicComponent` and `EngineTraceSimpleRecorder`, install them into `PlatformApplication`
 * first.
 *
 * Note that the difference from `EngineTraceSimpleRecorder` is that this component isn't dumb and offers a
 * complete solution to trace recording. Traces that were recorded with `startRecording` / `finishRecording` can
 * then be played back with `EngineTracePlayer`.
 *
 * Some notes on how this works. When starting trace recording:
 * - The game is saved.
 * - Then it is loaded right away. Loading is managed from the control thread.
 * - Then deterministic mode is entered, and actual event recording is started.
 *
 * Why do we need to start trace recording with loading the game? Loading the game updates the states of all actors
 * on the map, and since trace recording and playback must start in the same state, the only option that we have is to
 * start both trace playback and trace recording by loading the same save.
 *
 * @see EngineTracePlayer
 */
class EngineTraceRecorder : private PlatformApplicationAware {
 public:
    EngineTraceRecorder();
    ~EngineTraceRecorder();

    /**
     * Starts trace recording.
     *
     * Note that this method needs to be called from the control thread, see `EngineControlComponent`.
     *
     * @param game                      Engine controller.
     * @param savedGame                 Saved game to load instead of starting recording from current game state.
     */
    void startRecording(EngineController *game, const Blob &savedGame = {});

    /**
     * Finishes trace recording.
     *
     * Note that this method needs to be called from the control thread, see `EngineControlComponent`.
     *
     * @param game                      Engine controller.
     * @returns                         Resulting recording.
     */
    [[nodiscard]] EngineTraceRecording finishRecording(EngineController *game);

    /**
     * @return                          Whether recording is in progress. Make sure to call this method only from the
     *                                  control thread, otherwise you basically can't reason about the current recording
     *                                  state.
     */
    [[nodiscard]] bool isRecording() const {
        return _trace != nullptr;
    }

 private:
    friend class PlatformIntrospection;

 private:
    Blob _savedGame;
    std::unique_ptr<EventTrace> _trace;
    std::unique_ptr<ConfigPatch> _configSnapshot;
    std::unique_ptr<FileSystem> _ramFs;
    std::optional<ScopedRollback<FileSystem *>> _fsRollback;
};
