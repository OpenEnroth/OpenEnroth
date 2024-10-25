#pragma once

#include <cstdint>
#include <string>

#include "TileEnums.h"

struct TileData {
    std::string name;
    uint16_t uTileID = 0;
    Tileset tileset = Tileset_NULL;
    TILE_SECT uSection = TILE_SECT_Base1;
    TILE_DESC_FLAGS uAttributes = TILE_DESC_NULL;
};
