#pragma once

#include <vector>

#include "Engine/Data/TileData.h"

struct TileTable {
    TileData *GetTileById(unsigned int uTileID);
    int GetTileForTerrainType(Tileset a1, bool a2);
    unsigned int GetTileId(Tileset uTerrainType, TILE_SECT uSection);

    std::vector<TileData> tiles;
};

extern TileTable *pTileTable;
