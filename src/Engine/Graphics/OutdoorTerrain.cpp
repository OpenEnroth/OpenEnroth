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
int OutdoorTerrain::DoGetHeightOnTerrain(int x, int y) {
    if (x < 0 || x > 127 || y < 0 || y > 127)
        return 0;

    return 32 * pHeightmap[y * 128 + x];
}
