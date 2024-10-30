#pragma once

#include <vector>

#include "Engine/Data/TileData.h"

struct TileTable {
    int tileIdForTileset(TileSet tileset, bool nonRandom);
    int tileId(TileSet tileset, TileVariant section);

    std::vector<TileData> tiles; // Tile by id.
};

extern TileTable *pTileTable;
