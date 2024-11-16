#pragma once

#include <array>

#include "Library/Geometry/Point.h"
#include "Library/Image/Image.h"

#include "Engine/Data/TileEnums.h"

struct OutdoorLocation_MM7;

int GridCellToWorldPosX(int);
int GridCellToWorldPosY(int);
Pointi WorldPosToGrid(Vec3f worldPos);

class OutdoorTerrain {
 public:
    OutdoorTerrain();

    void CreateDebugTerrain();

    /**
     * @param gridPos                   Grid coordinates.
     * @return                          Terrain height at `gridPos`.
     * @offset 0x00488F2E, 0x0047EE16
     */
    int heightByGrid(Pointi gridPos) const;

    /**
     * @param pos                       World coordinates, only xy component is used by this function.
     * @return                          Terrain height at given position.
     * @offset 0x0048257A
     */
    int heightByPos(const Vec3f &pos) const;

    Vec3i vertexByGrid(Pointi gridPos) const {
        return Vec3i(GridCellToWorldPosX(gridPos.x), GridCellToWorldPosY(gridPos.y), heightByGrid(gridPos));
    }

    /**
     * @param gridPos                   Grid coordinates.
     * @return                          Tile id at `gridPos` that can then be used to get tile data from `TileTable`.
     */
    int tileIdByGrid(Pointi gridPos) const;

    /**
     * @param gridPos                   Grid coordinates.
     * @return                          Tile set for the tile at `gridPos`, or `Tileset_NULL` if the tile is invalid.
     */
    Tileset tilesetByGrid(Pointi gridPos) const;

    Tileset tilesetByPos(const Vec3f &pos) const;

    /**
     * @param gridPos                   Grid coordinates.
     * @return                          Whether the tile at `gridPos` is a water tile. Note that shore tiles are
     *                                  different from water tiles.
     */
    bool isWaterByGrid(Pointi gridPos) const;

    bool isWaterByPos(const Vec3f &pos) const;

    bool isWaterOrShoreByGrid(Pointi gridPos) const;

    /**
     * @param pos                       World coordinates, only xy component is used by this function.
     * @return                          Terrain normal at given position. Terrain normals always point up (`z > 0`).
     * @offset 0x0046DCC8
     */
    Vec3f normalByPos(const Vec3f &pos) const;

    const std::array<Vec3f, 2> &normalsByGrid(Pointi gridPos) const {
        return pTerrainNormals[gridPos];
    }

    /**
     * @param pos                       World coordinates, only xy component is used by this function.
     * @return                          Whether terrain slope at given position is too high to be climbed or stood on.
     * @offset 0x004823F4
     */
    bool isSlopeTooHighByPos(const Vec3f &pos) const;

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

    void recalculateNormals();
    TileGeometry tileGeometryByGrid(Pointi gridPos) const;

 private:
    std::array<Tileset, 4> pTileTypes; // Tileset ids used in this location, [3] is road tileset.
    Image<uint8_t> pHeightmap; // Height map, to get actual height multiply by 32.
    Image<int16_t> pTilemap; // Tile id map, indices into the global tile table.
    Image<std::array<Vec3f, 2>> pTerrainNormals; // Terrain normal map, two normals per tile for two triangles.
};
