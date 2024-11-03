#include "LOD.h"

#include <memory>

#include "Engine.h"
#include "EngineFileSystem.h"

std::unique_ptr<LodReader> pSave_LOD;
std::unique_ptr<LodReader> pGames_LOD;

bool Initialize_GamesLOD_NewLOD() {
    pGames_LOD = std::make_unique<LodReader>(dfs->read("data/games.lod"));
    pSave_LOD = std::make_unique<LodReader>();
    return true;
}
