#pragma once

#include <string>
#include <vector>

#include "Utility/Memory/Blob.h"

#include "TileEnums.h"

class GraphicsImage;

class TileDesc {
 public:
    std::string name;
    uint16_t uTileID = 0;
    Tileset tileset = Tileset_NULL;
    TILE_SECT uSection = TILE_SECT_Base1;
    TILE_DESC_FLAGS uAttributes = TILE_DESC_NULL;

    GraphicsImage *GetTexture();

 protected:
    GraphicsImage *texture = nullptr;
};

struct TileTable {
    TileDesc *GetTileById(unsigned int uTileID);
    int GetTileForTerrainType(Tileset a1, bool a2);
    unsigned int GetTileId(Tileset uTerrainType, TILE_SECT uSection);

    std::vector<TileDesc> tiles;
};

extern TileTable *pTileTable;
