#pragma once

#include <vector>

#include "Engine/Data/TileData.h"

struct TileTable {
    int tileIdForTileset(Tileset tileset, bool nonRandom);
    int tileId(Tileset tileset, TILE_SECT section);

    std::vector<TileData> tiles; // Tile by id.
};

extern TileTable *pTileTable;
