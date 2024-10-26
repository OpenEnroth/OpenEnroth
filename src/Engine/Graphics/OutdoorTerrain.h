#pragma once

#include <array>
#include <vector>

#include "Library/Geometry/Vec.h"

#include "Engine/Data/TileEnums.h"

struct OutdoorLocationTileType {
    Tileset tileset = Tileset_NULL;
    uint16_t uTileID = 0;
};

struct OutdoorTerrain {
    bool ZeroLandscape();
    void LoadBaseTileIds();
    void CreateDebugTerrain();

    int DoGetHeightOnTerrain(Vec2i gridPos);

    /**
     * @param gridPos                   Grid coordinates.
     * @return                          Tile id at `gridPos` that can then be used to get tile data from `TileTable`.
     */
    int tileId(Vec2i gridPos) const;

    /**
     * @param gridPos                   Grid coordinates.
     * @return                          Tile set for the tile at `gridPos`, or `Tileset_NULL` if the tile is invalid.
     */
    Tileset tileSet(Vec2i gridPos) const;

    int mapToGlobalTileId(int localTileId) const;

    std::array<OutdoorLocationTileType, 4> pTileTypes;  // [3] is road tileset.
    std::array<uint8_t, 128 * 128> pHeightmap = {};
    std::array<uint8_t, 128 * 128> pTilemap = {};
    std::array<uint8_t, 128 * 128> pAttributemap = {};
    std::vector<Vec3f> pTerrainNormals;
    std::array<unsigned short, 128 * 128 * 2> pTerrainNormalIndices;
};
