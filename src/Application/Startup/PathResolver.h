#pragma once

#include <string>
#include <vector>

#include "Engine/GameType.h"
#include "Library/FileSystem/Interface/FileSystemFwd.h"

#include "Utility/System/Path.h"

class Environment;

constexpr char mm6PathOverrideKey[] = "OPENENROTH_MM6_PATH";
constexpr char mm7PathOverrideKey[] = "OPENENROTH_MM7_PATH";
constexpr char mm8PathOverrideKey[] = "OPENENROTH_MM8_PATH";

std::vector<Path> resolveMm6Paths(Environment *environment);
std::vector<Path> resolveMm7Paths(Environment *environment);
std::vector<Path> resolveMm8Paths(Environment *environment);

bool validateDataPath(PathView dataPath, std::string *missingFile);

/**
 * Detects the game version of a data folder from files that are unique to each game's layout (see
 * `validateDataPath`'s validate lists). This is the single detector for the engine-wide `GameType` seam - callers
 * should not duplicate this logic, but call this function (or `setGameType`+this function, as `GameStarter` does
 * once at boot) instead.
 *
 * @param fs                            Filesystem rooted at the data folder to check.
 * @return                              `GAME_TYPE_MM8` if `fs` looks like an MM8 data folder, `GAME_TYPE_MM7`
 *                                      otherwise (including when detection is ambiguous - MM7 is the default).
 */
GameType detectGameType(FileSystem &fs);

/**
 * @param dataPath                     Path to a data folder on disk.
 * @return                              Same as `detectGameType(FileSystem &)`, but for a data folder on disk.
 */
GameType detectGameType(PathView dataPath);

Path resolveMm7UserPath(Environment *environment);
