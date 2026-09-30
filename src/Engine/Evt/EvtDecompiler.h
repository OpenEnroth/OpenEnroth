#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

class EvtProgram;

/**
 * The Lua script of an evt file, in one piece for each event.
 */
struct EvtLuaScript {
    std::string header; // Puts the map's strings into `evt.str` and removes the file's events.
    std::vector<std::pair<int, std::string>> events; // Id and code of each event, by id.

    /**
     * @return                          The whole script.
     */
    std::string text() const;
};

/**
 * Turns an evt file into a Lua script for MMExtension's `evt` API that does what the file does. The script starts by
 * removing the file's events, so it takes their place when it's loaded as the map's script.
 *
 * Conditions become `if` blocks where no jump enters the block from outside, and `goto` otherwise. Names from the game
 * tables go into comments when the tables are loaded.
 *
 * @param program                       The evt file.
 * @param strings                       Strings of the map, from its str file. Empty for global.evt.
 * @param isGlobal                      Whether this is global.evt.
 * @return                              The script.
 */
EvtLuaScript decompileEvt(const EvtProgram &program, const std::vector<std::string> &strings, bool isGlobal);

/**
 * @param name                          Name of an evt file of the game without the extension, e.g. "d27" or "global".
 * @return                              The file as a Lua script, see `decompileEvt`.
 * @throws Exception                    If the game has no such file.
 */
EvtLuaScript decompileGameEvt(std::string_view name);
