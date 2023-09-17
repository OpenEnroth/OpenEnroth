#include "LOD.h"

#include "Utility/DataPath.h"

std::unique_ptr<LodReader> pSave_LOD; // LOD pointing to the savegame file currently being processed
std::unique_ptr<LodReader> pGames_LOD; // LOD pointing to data/games.lod

bool Initialize_GamesLOD_NewLOD() {
    pGames_LOD = std::make_unique<LodReader>(makeDataPath("data", "games.lod"));
    pSave_LOD = std::make_unique<LodReader>();
    return true;
}
