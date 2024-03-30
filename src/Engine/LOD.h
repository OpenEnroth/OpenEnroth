#pragma once

#include <memory>

#include "Library/Lod/LodReader.h"

bool Initialize_GamesLOD_NewLOD();

/** LOD reader for the current game being played. Used for accessing the per-map states, is updated on map change,
 * on save, and on load. */
extern std::unique_ptr<LodReader> pSave_LOD;

/** LOD reader for data/games.lod. */
extern std::unique_ptr<LodReader> pGames_LOD;
