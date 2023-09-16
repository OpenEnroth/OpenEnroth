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
    uint16_t uSection = 0;
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
    int GetTileForTerrainType(signed int a1, bool a2);
    unsigned int GetTileId(unsigned int uTerrainType, unsigned int uSection);
    int FromFileTxt(const char *pFilename);

    std::vector<TileDesc> tiles;
};

extern struct TileTable *pTileTable;
