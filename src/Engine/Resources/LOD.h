#pragma once

#include <memory>

#include "Library/Lod/LodReader.h"

class FileSystem;

bool Initialize_GamesLOD_NewLOD();

/** LOD reader for data/games.lod. */
extern std::unique_ptr<LodReader> pGames_LOD;
