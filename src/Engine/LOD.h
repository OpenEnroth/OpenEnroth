#pragma once

#include <memory>

#include "Library/Lod/LodReader.h"

bool Initialize_GamesLOD_NewLOD();

extern std::unique_ptr<LodReader> pSave_LOD;
extern std::unique_ptr<LodReader> pGames_LOD;
