#pragma once

#include <cstdint>
#include <string>

#include "TileEnums.h"

struct TileData {
    std::string name;
    uint16_t uTileID = 0;
    Tileset tileset = TILESET_INVALID;
    TileVariant uSection = TILE_VARIANT_BASE1;
    TileFlags uAttributes;
};
