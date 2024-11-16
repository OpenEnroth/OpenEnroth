#pragma once

#include <array>

#include "Library/Geometry/Vec.h"
#include "Library/Image/Image.h"

#include "Engine/Data/TileEnums.h"

struct OutdoorLocation_MM7;

class OutdoorTerrain {
 public:
    OutdoorTerrain();

    void CreateDebugTerrain();

    /**
     * @param gridPos                   Grid coordinates.
     * @return                          Terrain height at `gridPos`.
     * @offset 0x00488F2E, 0x0047EE16
     */
    int heightByGrid(Vec2i gridPos) const;

    /**
     * @offset 0x0048257A
     */
    int heightByPos(const Vec3f &pos) const;

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

    TileSet tileSetByPos(const Vec3f &pos) const;

    /**
     * @param gridPos                   Grid coordinates.
     * @return                          Whether the tile at `gridPos` is a water tile. Note that shore tiles are
     *                                  different from water tiles.
     */
    bool isWaterByGrid(Vec2i gridPos) const;

    bool isWaterOrShoreByGrid(Vec2i gridPos) const;

    bool isWaterByPos(const Vec3f &pos) const;

    /**
     * @param pos                       World coordinates, only xy component is used by this function.
     * @return                          Terrain normal at given position. Terrain normals always point up (`z > 0`).
     * @offset 0x0046DCC8
     */
    Vec3f normalByPos(const Vec3f &pos) const;

    /**
     * @param pos                       World coordinates, only xy component is used by this function.
     * @return                          Whether terrain slope at given position is too high to be climbed or stood on.
     * @offset 0x004823F4
     */
    bool isSlopeTooHighByPos(const Vec3f &pos) const;

    std::array<TileSet, 4> pTileTypes; // Tileset ids used in this location, [3] is road tileset.
    Image<uint8_t> pHeightmap; // Height map, to get actual height multiply by 32.
    Image<int16_t> pTilemap; // Tile id map, indices into the global tile table.
    Image<std::array<Vec3f, 2>> pTerrainNormals; // Terrain normal map, two normals per tile for two triangles.

    friend void reconstruct(const OutdoorLocation_MM7 &src, OutdoorTerrain *dst);

 private:
    struct TileGeometry {
        int x0 = 0;
        int x1 = 0;
        int y0 = 0;
        int y1 = 0; // We have a retarded coordinate system, so y1 < y0, always.
        int z00 = 0;
        int z01 = 0;
        int z10 = 0;
        int z11 = 0;
    };

 private:
    void recalculateNormals();
    TileGeometry tileGeometryByGrid(Vec2i gridPos) const;
};
