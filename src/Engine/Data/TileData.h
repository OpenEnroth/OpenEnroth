#pragma once

#include <cstdint>
#include <string>

#include "TileEnums.h"

struct TileData {
    std::string name;
    Tileset tileset = TILESET_INVALID;
    TileVariant variant = TILE_VARIANT_BASE1;
    TileFlags flags;
};
