#pragma once

#include <array>
#include <vector>

#include "Library/Geometry/Vec.h"

#include "Engine/Data/TileEnums.h"

#include "Media/Audio/SoundEnums.h"

struct OutdoorLocationTileType {
    TileSet tileset = TILE_SET_INVALID;
    uint16_t uTileID = 0;
};

class OutdoorTerrain {
 public:
    bool ZeroLandscape();
    void LoadBaseTileIds();
    void CreateDebugTerrain();

    int heightByGrid(Vec2i gridPos);

    /**
     * @param gridPos                   Grid coordinates.
     * @return                          Tile id at `gridPos` that can then be used to get tile data from `TileTable`.
     */
    int tileIdByGrid(Vec2i gridPos) const;

    /**
     * @param gridPos                   Grid coordinates.
     * @return                          Tile set for the tile at `gridPos`, or `Tileset_NULL` if the tile is invalid.
     */
    TileSet tileSetByGrid(Vec2i gridPos) const;

    /**
     * @offset 0x47EE49
     */
    SoundId soundIdByGrid(Vec2i gridPos, bool isRunning);

    std::array<OutdoorLocationTileType, 4> pTileTypes;  // [3] is road tileset.
    std::array<uint8_t, 128 * 128> pHeightmap = {};
    std::array<uint8_t, 128 * 128> pTilemap = {};
    std::array<uint8_t, 128 * 128> pAttributemap = {};
    std::vector<Vec3f> pTerrainNormals;
    std::array<unsigned short, 128 * 128 * 2> pTerrainNormalIndices;

 private:
    int mapToGlobalTileId(int localTileId) const;
};
