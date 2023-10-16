#pragma once

#include <string>
#include <vector>

#include "Utility/Memory/Blob.h"

#include "TileEnums.h"

class GraphicsImage;

class TileDesc {
 public:
    inline TileDesc() : texture(nullptr) {}

    std::string name;
    uint16_t uTileID = 0;
    Tileset tileset = Tileset_NULL;
    TILE_SECT uSection = TILE_SECT_Base1;
    TILE_DESC_FLAGS uAttributes = TILE_DESC_NULL;

    GraphicsImage *GetTexture();

    // inline bool IsWaterTile() const         { return this->name == "wtrtyl";
    // }
    inline bool IsWaterTile() const { return this->uAttributes & TILE_DESC_WATER; }
    inline bool IsWaterBorderTile() const {
        return this->name.find("wtrdr", 0) == 0 ||
               this->name.find("hwtrdr") == 0;
    }

 protected:
    GraphicsImage *texture;
};

struct TileTable {
    TileDesc *GetTileById(unsigned int uTileID);
    void InitializeTileset(Tileset tileset);
    int GetTileForTerrainType(Tileset a1, bool a2);
    unsigned int GetTileId(Tileset uTerrainType, TILE_SECT uSection);

    std::vector<TileDesc> tiles;
};

extern struct TileTable *pTileTable;
