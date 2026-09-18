#pragma once

#include <span>
#include <string>
#include <vector>

#include "Engine/Evt/EvtCommands.h"

/**
 * Turns an evt file into a Lua script for MMExtension's `evt` API that does what the file does. The script starts by
 * removing the file's events, so it takes their place when it's loaded as the map's script.
 *
 * Conditions become `if` blocks where no jump enters the block from outside, and `goto` otherwise. Names from the game
 * tables go into comments when the tables are loaded.
 *
 * @param records                       Records of the evt file, see `decodeEvtRecords`.
 * @param strings                       Strings of the map, from its str file. Empty for global.evt.
 * @param isGlobal                      Whether this is global.evt.
 * @return                              The script.
 */
std::string decompileEvt(std::span<const EvtRecord> records, const std::vector<std::string> &strings, bool isGlobal);

/**
 * @param name                          Name of an evt file of the game without the extension, e.g. "d27" or "global".
 * @param skippedEvents                 Ids of the events to leave out.
 * @return                              The file as a Lua script, see `decompileEvt`.
 * @throws Exception                    If the game has no such file.
 */
std::string decompileGameEvt(std::string_view name, std::span<const int> skippedEvents = {});
