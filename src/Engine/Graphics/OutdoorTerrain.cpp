#include "OutdoorTerrain.h"

#include "Engine/Tables/TileTable.h"

//----- (0047CCE2) --------------------------------------------------------
bool OutdoorTerrain::ZeroLandscape() {
    this->pHeightmap.fill(0);
    this->pTilemap.fill(90);
    this->pAttributemap.fill(0);
    return true;
}

//----- (0047F420) --------------------------------------------------------
void OutdoorTerrain::LoadBaseTileIds() {
    for (unsigned i = 0; i < 3; ++i)
        pTileTypes[i].uTileID = pTileTable->tileIdForTileset(pTileTypes[i].tileset, 1);
}

void OutdoorTerrain::CreateDebugTerrain() {
    ZeroLandscape();
    pTileTypes[0].tileset = Tileset_Grass;
    pTileTypes[1].tileset = Tileset_Water;
    pTileTypes[2].tileset = Tileset_Badlands;
    pTileTypes[3].tileset = Tileset_RoadGrassCobble;
    LoadBaseTileIds();
}

//----- (00488F2E) --------------------------------------------------------
//----- (0047EE16) --------------------------------------------------------
int OutdoorTerrain::heightByGrid(Vec2i gridPos) {
    if (gridPos.x < 0 || gridPos.x > 127 || gridPos.y < 0 || gridPos.y > 127)
        return 0;

    return 32 * pHeightmap[gridPos.y * 128 + gridPos.x];
}

int OutdoorTerrain::tileIdByGrid(Vec2i gridPos) const {
    if (gridPos.x < 0 || gridPos.x > 127 || gridPos.y < 0 || gridPos.y > 127)
        return 0;

    return mapToGlobalTileId(pTilemap[gridPos.y * 128 + gridPos.x]);
}

Tileset OutdoorTerrain::tileSetByGrid(Vec2i gridPos) const {
    if (gridPos.x < 0 || gridPos.x > 127 || gridPos.y < 0 || gridPos.y > 127)
        return Tileset_NULL;

    int localTileId = pTilemap[gridPos.y * 128 + gridPos.x];

    if (localTileId >= 1 && localTileId <= 12)
        return Tileset_Dirt; // See comment in mapToGlobalTileId.

    if (localTileId >= 234 || localTileId < 90)
        return Tileset_NULL;

    int tileSetIndex = (localTileId - 90) / 36;
    return pTileTypes[tileSetIndex].tileset;
}

SoundId OutdoorTerrain::soundIdByGrid(Vec2i gridPos, bool isRunning) {
    // TODO(captainurist): this doesn't take seasons into account.
    switch (tileSetByGrid(gridPos)) {
    case Tileset_Grass:
        return isRunning ? SOUND_RunGrass : SOUND_WalkGrass;
    case Tileset_Snow:
        return isRunning ? SOUND_RunSnow : SOUND_WalkSnow;
    case Tileset_Desert:
        return isRunning ? SOUND_RunDesert : SOUND_WalkDesert;
    case Tileset_CooledLava:
        return isRunning ? SOUND_RunCooledLava : SOUND_WalkCooledLava;
    case Tileset_NULL: // Use dirt sounds for invalid tiles.
    case Tileset_Dirt:
        // Water sounds were used
        return isRunning ? SOUND_RunDirt : SOUND_WalkDirt;
    case Tileset_Water:
        // Dirt sounds were used
        return isRunning ? SOUND_RunWater : SOUND_WalkWater;
    case Tileset_Badlands:
        return isRunning ? SOUND_RunBadlands : SOUND_WalkBadlands;
    case Tileset_Swamp:
        return isRunning ? SOUND_RunSwamp : SOUND_WalkSwamp;
    case Tileset_Tropical:
        // TODO(Nik-RE-dev): is that correct?
        return isRunning ? SOUND_RunGrass : SOUND_WalkGrass;
    case Tileset_RoadGrassCobble:
    case Tileset_RoadGrassDirt:
    case Tileset_RoadSnowCobble:
    case Tileset_RoadSnowDirt:
    case Tileset_RoadSandCobble:
    case Tileset_RoadSandDirt:
    case Tileset_RoadVolcanoCobble:
    case Tileset_RoadVolcanoDirt:
    case Tileset_RoadCrackedCobble:
    case Tileset_RoadCrackedDirt:
    case Tileset_RoadSwampCobble:
    case Tileset_RoadSwampDir:
    case Tileset_RoadTropicalCobble:
    case Tileset_RoadTropicalDirt:
        return isRunning ? SOUND_RunRoad : SOUND_WalkRoad;
    case Tileset_City:
    case Tileset_RoadCityStone:
        // TODO(Nik-RE-dev): is that correct?
    default:
        return isRunning ? SOUND_RunGround : SOUND_WalkGround;
    }
}

int OutdoorTerrain::mapToGlobalTileId(int localTileId) const {
    // Tiles in tilemap:
    // [0..90) are mapped as-is, but seem to be mostly invalid. Only global tile ids [1..12] are valid (all are dirt),
    //         the rest are "pending", effectively invalid.
    // [90..126) map to tileset #1.
    // [126..162) map to tileset #2.
    // [162..198) map to tileset #3.
    // [198..234) map to tileset #4 (road).
    // [234..255) are invalid.

    if (localTileId < 90)
        return localTileId;

    if (localTileId >= 234)
        return 0;

    int tileSetIndex = (localTileId - 90) / 36;
    int tileSetOffset = (localTileId - 90) % 36;
    return pTileTypes[tileSetIndex].uTileID + tileSetOffset;
}
