#pragma once

#include <array>

#include "Engine/Data/TileEnums.h"

struct OutdoorLocationTileType {
    Tileset tileset = Tileset_NULL;
    uint16_t uTileID = 0;
};

struct OutdoorTerrain {
    bool ZeroLandscape();

    std::array<OutdoorLocationTileType, 4> pTileTypes;  // [3] is road tileset.
    std::array<uint8_t, 128 * 128> pHeightmap = {};
    std::array<uint8_t, 128 * 128> pTilemap = {};
    std::array<uint8_t, 128 * 128> pAttributemap = {};
};
