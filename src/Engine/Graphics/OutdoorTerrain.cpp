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
int OutdoorTerrain::DoGetHeightOnTerrain(Vec2i gridPos) {
    if (gridPos.x < 0 || gridPos.x > 127 || gridPos.y < 0 || gridPos.y > 127)
        return 0;

    return 32 * pHeightmap[gridPos.y * 128 + gridPos.x];
}

int OutdoorTerrain::tileId(int x, int y) const {
    if (x < 0 || x > 127 || y < 0 || y > 127)
        return 0;

    return mapToGlobalTileId(pTilemap[y * 128 + x]);
}

Tileset OutdoorTerrain::tileSet(int x, int y) const {
    if (x < 0 || x > 127 || y < 0 || y > 127)
        return Tileset_NULL;

    int localTileId = pTilemap[y * 128 + x];

    if (localTileId >= 1 && localTileId <= 12)
        return Tileset_Dirt; // See comment in mapToGlobalTileId.

    if (localTileId >= 234 || localTileId < 90)
        return Tileset_NULL;

    int tileSetIndex = (localTileId - 90) / 36;
    return pTileTypes[tileSetIndex].tileset;
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
