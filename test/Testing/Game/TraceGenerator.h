#pragma once

#include <functional>
#include <string_view>

class EngineController;
class FileSystem;

/**
 * Programmatic trace-generation helper.
 *
 * Drives `routine` against `game` while trace recording (`EngineTraceRecorder::startRecording` /
 * `finishRecording`) is active, then writes the resulting trace + save as `<baseName>.json` and `<baseName>.mm7`
 * into `fs`.
 *
 * This is the programmatic equivalent of the Ctrl+Shift+R hotkey (`GameTraceHandler`). Pass e.g. a
 * `DirectoryFileSystem` pointed at `test/Data` to write the trace straight into the repo's test data.
 *
 * @param game                          Engine controller to drive.
 * @param fs                            Filesystem to write the trace + save into.
 * @param baseName                       Base file name, without extension.
 * @param routine                        Routine to run while recording. Runs on the control thread, just like a
 *                                       `GAME_TEST` body - use `game` to drive the engine (`teleportTo`,
 *                                       `spawnMonster`, `castSpell`, `tick`, etc).
 */
void generateTrace(EngineController &game, FileSystem *fs, std::string_view baseName,
                   const std::function<void()> &routine);
