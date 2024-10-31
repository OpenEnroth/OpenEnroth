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
    pTileTypes[0].tileset = TILE_SET_GRASS;
    pTileTypes[1].tileset = TILE_SET_WATER;
    pTileTypes[2].tileset = TILE_SET_BADLANDS;
    pTileTypes[3].tileset = TILE_SET_ROAD_GRASS_COBBLE;
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

TileSet OutdoorTerrain::tileSetByGrid(Vec2i gridPos) const {
    if (gridPos.x < 0 || gridPos.x > 127 || gridPos.y < 0 || gridPos.y > 127)
        return TILE_SET_INVALID;

    int localTileId = pTilemap[gridPos.y * 128 + gridPos.x];

    if (localTileId >= 1 && localTileId <= 12)
        return TILE_SET_DIRT; // See comment in mapToGlobalTileId.

    if (localTileId >= 234 || localTileId < 90)
        return TILE_SET_INVALID;

    int tileSetIndex = (localTileId - 90) / 36;
    return pTileTypes[tileSetIndex].tileset;
}

SoundId OutdoorTerrain::soundIdByGrid(Vec2i gridPos, bool isRunning) {
    // TODO(captainurist): this doesn't take seasons into account.
    switch (tileSetByGrid(gridPos)) {
    case TILE_SET_GRASS:
        return isRunning ? SOUND_RunGrass : SOUND_WalkGrass;
    case TILE_SET_SNOW:
        return isRunning ? SOUND_RunSnow : SOUND_WalkSnow;
    case TILE_SET_DESERT:
        return isRunning ? SOUND_RunDesert : SOUND_WalkDesert;
    case TILE_SET_COOLED_LAVA:
        return isRunning ? SOUND_RunCooledLava : SOUND_WalkCooledLava;
    case TILE_SET_INVALID: // Use dirt sounds for invalid tiles.
    case TILE_SET_DIRT:
        // Water sounds were used
        return isRunning ? SOUND_RunDirt : SOUND_WalkDirt;
    case TILE_SET_WATER:
        // Dirt sounds were used
        return isRunning ? SOUND_RunWater : SOUND_WalkWater;
    case TILE_SET_BADLANDS:
        return isRunning ? SOUND_RunBadlands : SOUND_WalkBadlands;
    case TILE_SET_SWAMP:
        return isRunning ? SOUND_RunSwamp : SOUND_WalkSwamp;
    case TILE_SET_TROPICAL:
        // TODO(Nik-RE-dev): is that correct?
        return isRunning ? SOUND_RunGrass : SOUND_WalkGrass;
    case TILE_SET_ROAD_GRASS_COBBLE:
    case TILE_SET_ROAD_GRASS_DIRT:
    case TILE_SET_ROAD_SNOW_COBBLE:
    case TILE_SET_ROAD_SNOW_DIRT:
    case TILE_SET_ROAD_SAND_COBBLE:
    case TILE_SET_ROAD_SAND_DIRT:
    case TILE_SET_ROAD_VOLCANO_COBBLE:
    case TILE_SET_ROAD_VOLCANO_DIRT:
    case TILE_SET_ROAD_CRACKED_COBBLE:
    case TILE_SET_ROAD_CRACKED_DIRT:
    case TILE_SET_ROAD_SWAMP_COBBLE:
    case TILE_SET_ROAD_SWAMP_DIRT:
    case TILE_SET_ROAD_TROPICAL_COBBLE:
    case TILE_SET_ROAD_TROPICAL_DIRT:
        return isRunning ? SOUND_RunRoad : SOUND_WalkRoad;
    case TILE_SET_CITY:
    case TILE_SET_ROAD_CITY_STONE:
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
