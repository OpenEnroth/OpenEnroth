#pragma once

#include <array>

#include "Library/Geometry/Point.h"
#include "Library/Image/Image.h"

#include "Engine/Data/TileEnums.h"

struct TileData;
struct OutdoorLocation_MM7;

/**
 * A note on grid coordinates. In grid coordinates Y-axis points south (down on the minimap), X-axis points east (right
 * on the minimap), `(0, 0)` is NW (top-left) corner of the map.
 *
 * @param gridPos                       Grid coordinates.
 * @return                              World coordinates of the grid cell's NW (top-left on the minimap) corner.
 * @offset 0x0047F469, 0x0047F476
 */
inline Vec2i gridToWorld(Pointi gridPos) {
    return {(gridPos.x - 64) << 9, (64 - gridPos.y) << 9};
}

/**
 * A note on world coordinates. Y-axis points north (up on the minimap), X-axis points east (right on the minimap),
 * `(0, 0)` is in the center of the map.
 *
 * @param worldPos                      Position in world coordinates.
 * @return                              Grid cell coordinates that `worldPos` is in.
 * @offset 0x0047F44B, 0x0047F458
 */
inline Pointi worldToGrid(const Vec3f &worldPos) {
    int worldX = worldPos.x;
    int worldY = worldPos.y;

    // sar is in original exe, resulting -880 / 512 = -1 and -880 sar 9 = -2.
    int gridX = (worldX >> 9) + 64;
    int gridY = 63 - (worldY >> 9);
    return Pointi(gridX, gridY);
}

/**
 * Terrain for outdoor location.
 *
 * Contains geometry & tile data, and provides some convenience methods for accessing it.
 *
 * All methods do bounds checking, so this class effectively presents a view into a location that's infinite in size.
 * Methods ending with `-Unsafe` don't do bounds checking.
 */
class OutdoorTerrain {
 public:
    OutdoorTerrain();

    void createDebugTerrain();

    void changeSeason(int month);

    /**
     * @param gridPos                   Grid coordinates.
     * @return                          Terrain height at `gridPos`.
     * @offset 0x00488F2E, 0x0047EE16
     */
    [[nodiscard]] int heightByGrid(Pointi gridPos) const;

    /**
     * @param pos                       World coordinates, only xy component is used by this function.
     * @return                          Terrain height at given position.
     * @offset 0x0048257A
     */
    [[nodiscard]] int heightByPos(const Vec3f &pos) const;

    [[nodiscard]] Vec3i vertexByGridUnsafe(Pointi gridPos) const {
        Vec2i tmp = gridToWorld(gridPos);
        return Vec3i(tmp.x, tmp.y, 32 * _heightMap[gridPos]);
    }

    /**
     * @param gridPos                   Grid coordinates.
     * @return                          Tile id at `gridPos` that can then be used to get tile data from `TileTable`.
     */
    [[nodiscard]] int tileIdByGrid(Pointi gridPos) const;

    /**
     * @param gridPos                   Grid coordinates.
     * @return                          Tile data from the global tile table for the tile at `gridPos`.
     * @offset 0x47ED08
     */
    [[nodiscard]] const TileData &tileDataByGrid(Pointi gridPos) const;

    /**
     * @param gridPos                   Grid coordinates.
     * @return                          Tile set for the tile at `gridPos`, or `Tileset_NULL` if the tile is invalid.
     */
    [[nodiscard]] Tileset tilesetByGrid(Pointi gridPos) const;

    [[nodiscard]] Tileset tilesetByPos(const Vec3f &pos) const;

    /**
     * @param gridPos                   Grid coordinates.
     * @return                          Whether the tile at `gridPos` is a water tile. Note that shore tiles are
     *                                  different from water tiles.
     */
    [[nodiscard]] bool isWaterByGrid(Pointi gridPos) const;

    [[nodiscard]] bool isWaterByPos(const Vec3f &pos) const;

    [[nodiscard]] bool isWaterOrShoreByGrid(Pointi gridPos) const;

    /**
     * @param pos                       World coordinates, only xy component is used by this function.
     * @return                          Terrain normal at given position. Terrain normals always point up (`z > 0`).
     * @offset 0x0046DCC8
     */
    [[nodiscard]] Vec3f normalByPos(const Vec3f &pos) const;

    [[nodiscard]] const std::array<Vec3f, 2> &normalsByGridUnsafe(Pointi gridPos) const {
        return _normalMap[gridPos];
    }

    /**
     * @param pos                       World coordinates, only xy component is used by this function.
     * @return                          Whether terrain slope at given position is too high to be climbed or stood on.
     * @offset 0x004823F4
     */
    [[nodiscard]] bool isSlopeTooHighByPos(const Vec3f &pos) const;

    friend void reconstruct(const OutdoorLocation_MM7 &src, OutdoorTerrain *dst); // In CompositeSnapshots.cpp.

 private:
    struct TileGeometry {
        Vec2i v0;
        Vec2i v1; // We have a retarded coordinate system, so v1.y < v0.y, always.
        int z00 = 0;
        int z01 = 0;
        int z10 = 0;
        int z11 = 0;
    };

    void recalculateNormals();
    [[nodiscard]] TileGeometry tileGeometryByGrid(Pointi gridPos) const;

 private:
    std::array<Tileset, 4> _tilesets; // Tileset ids used in this location, [3] is road tileset.
    Image<uint8_t> _heightMap; // Height map, to get actual height multiply by 32.
    Image<int16_t> _tileMap; // Tile id map, indices into the global tile table.
    Image<int16_t> _originalTileMap; // Same as above, but w/o seasonal changes.
    Image<std::array<Vec3f, 2>> _normalMap; // Terrain normal map, two normals per tile for two triangles.
};
