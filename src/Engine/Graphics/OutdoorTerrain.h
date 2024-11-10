#pragma once

#include <array>
#include <vector>

#include "Library/Geometry/Vec.h"

#include "Engine/Data/TileEnums.h"

#include "Media/Audio/SoundEnums.h"

struct OutdoorTileType {
    TileSet tileset = TILE_SET_INVALID;
    uint16_t uTileID = 0;
};

struct OutdoorTileGeometry {
    Vec3f v00, v01, v10, v11; // Four vertices of the tile, v00 is at (x0, y0), v01 at (x0, y1), etc.
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
    SoundId soundIdByGrid(Vec2i gridPos, bool isRunning) const;

    /**
     *
     * @param pos                       World coordinates, only xy component is used by this function.
     * @return                          Terrain normal at given position. Terrain normals always point up (`z > 0`).
     * @offset 0x0046DCC8
     */
    Vec3f normalByPos(const Vec3f& pos) const;

    /**
     * @param pos                       World coordinates, only xy component is used by this function.
     * @return                          Whether terrain slope at given position is too high to be climbed or stood on.
     * @offset 0x004823F4
     */
    bool isSlopeTooHighByPos(const Vec3f& pos) const;

    // TODO(captainurist): also move all the functions that use this method into this class.
    OutdoorTileGeometry tileGeometryByGrid(Vec2i gridPos) const;

    std::array<OutdoorTileType, 4> pTileTypes;  // [3] is road tileset.
    std::array<uint8_t, 128 * 128> pHeightmap = {};
    std::array<uint8_t, 128 * 128> pTilemap = {};
    std::array<uint8_t, 128 * 128> pAttributemap = {};
    std::vector<Vec3f> pTerrainNormals;
    std::array<unsigned short, 128 * 128 * 2> pTerrainNormalIndices;

 private:
    int mapToGlobalTileId(int localTileId) const;
};
