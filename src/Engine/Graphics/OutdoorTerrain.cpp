#include "OutdoorTerrain.h"

#include <cassert>
#include <algorithm>

#include "Engine/Tables/TileTable.h"
#include "Engine/Snapshots/CompositeSnapshots.h"
#include "Engine/Snapshots/EntitySnapshots.h"

#include "Library/Snapshots/CommonSnapshots.h"

#include "Outdoor.h"

static int mapToGlobalTileId(const std::array<int, 4> &baseIds, int localTileId) {
    // Tiles in tilemap:
    // [0..90) are mapped as-is, but seem to be mostly invalid. Only global tile ids [1..12] are valid (all are dirt),
    //         the rest are "pending", effectively invalid.
    // [90..126) map to tileset #1.
    // [126..162) map to tileset #2.
    // [162..198) map to tileset #3.
    // [198..234) map to tileset #4 (road).
    // [234..255) are invalid.
    if (localTileId < 90)
        return localTileId;

    if (localTileId >= 234)
        return 0;

    int tilesetIndex = (localTileId - 90) / 36;
    int tilesetOffset = (localTileId - 90) % 36;
    return baseIds[tilesetIndex] + tilesetOffset;
}

template<class Image>
static bool contains(const Image &image, Pointi point) {
    return point.x >= 0 && point.x < image.width() && point.y >= 0 && point.y <= image.height();
}

//----- (0047F44B) --------------------------------------------------------
//----- (0047F458) --------------------------------------------------------
Pointi WorldPosToGrid(Vec3f worldPos) {
    int worldX = worldPos.x;
    int worldY = worldPos.y;

    // sar is in original exe, resulting -880 / 512 = -1 and -880 sar 9 = -2.
    int gridX = (worldX >> 9) + 64;
    int gridY = 63 - (worldY >> 9);
    return Pointi(gridX, gridY);
}

//----- (0047F469) --------------------------------------------------------
int GridCellToWorldPosX(int a1) { return (a1 - 64) << 9; }

//----- (0047F476) --------------------------------------------------------
int GridCellToWorldPosY(int a1) { return (64 - a1) << 9; }

OutdoorTerrain::OutdoorTerrain() {
    // Map is 127x127 squares.
    pHeightmap = Image<uint8_t>::solid(128, 128, 0);
    pTilemap = Image<int16_t>::solid(127, 127, 0);
    pTerrainNormals = Image<std::array<Vec3f, 2>>::solid(127, 127, {Vec3f(0, 0, 1), Vec3f(0, 0, 1)});
}

void OutdoorTerrain::CreateDebugTerrain() {
    int tileId = pTileTable->tileId(TILESET_GRASS, TILE_VARIANT_BASE1);

    pHeightmap.fill(0);
    pTilemap.fill(tileId);
    pTerrainNormals.fill({Vec3f(0, 0, 1), Vec3f(0, 0, 1)});

    pTileTypes[0] = TILESET_GRASS;
    pTileTypes[1] = TILESET_WATER;
    pTileTypes[2] = TILESET_BADLANDS;
    pTileTypes[3] = TILESET_ROAD_GRASS_COBBLE;
}

int OutdoorTerrain::heightByGrid(Pointi gridPos) const {
    if (!contains(pHeightmap, gridPos))
        return 0;

    return 32 * pHeightmap[gridPos];
}

int OutdoorTerrain::heightByPos(const Vec3f &pos) const {
    // TODO(captainurist): This should return float. But we'll need to retrace.
    int originz;          // ebx@11
    int lz;          // eax@11
    int rz;         // ecx@11
    int rpos;         // [sp+10h] [bp-8h]@11
    int lpos;         // [sp+24h] [bp+Ch]@11

    // TODO(captainurist): this function had some code that would push the party -60 units down when on a water tile AND
    //                     not water-walking, but this isn't enabled in the game. I tried it, and it actually looks
    //                     good, as if the party is actually a bit submerged and swimming. The only problem is that
    //                     party would be jerked up upon coming ashore, and this just looks ugly. Find a way to
    //                     reimplement this properly.

    Pointi gridPos = WorldPosToGrid(pos);

    TileGeometry tile = tileGeometryByGrid(gridPos);

    if (tile.z00 != tile.z10 || tile.z10 != tile.z11 || tile.z11 != tile.z01) {
        // On a slope.
        if (std::abs(tile.y0 - pos.y) >= std::abs(pos.x - tile.x0)) {
            originz = tile.z01;
            lz = tile.z11;
            rz = tile.z00;
            lpos = pos.x - tile.x0;
            rpos = pos.y - tile.y1;
        } else {
            originz = tile.z10;
            lz = tile.z00;
            rz = tile.z11;
            lpos = tile.x1 - pos.x;
            rpos = tile.y0 - pos.y;
        }

        //assert(lpos >= 0 && lpos < 512); // TODO(captainurist): fails in rare cases b/c not all of our code is in floats
        //assert(rpos >= 0 && rpos < 512);

        // (x >> 9) is basically (x / 512) but with consistent rounding towards -inf.
        return originz + ((rpos * (rz - originz)) >> 9) + ((lpos * (lz - originz)) >> 9);
    } else {
        // On flat terrain.
        return tile.z00;
    }
}

int OutdoorTerrain::tileIdByGrid(Pointi gridPos) const {
    if (!contains(pTilemap, gridPos))
        return 0;

    return pTilemap[gridPos];
}

Tileset OutdoorTerrain::tilesetByGrid(Pointi gridPos) const {
    if (!contains(pTilemap, gridPos))
        return TILESET_INVALID;

    return pTileTable->tiles[pTilemap[gridPos]].tileset;
}

Tileset OutdoorTerrain::tilesetByPos(const Vec3f &pos) const {
    return tilesetByGrid(WorldPosToGrid(pos));
}

bool OutdoorTerrain::isWaterByGrid(Pointi gridPos) const {
    return pTileTable->tiles[tileIdByGrid(gridPos)].uAttributes & TILE_WATER;
}

bool OutdoorTerrain::isWaterByPos(const Vec3f &pos) const {
    return isWaterByGrid(WorldPosToGrid(pos));
}

bool OutdoorTerrain::isWaterOrShoreByGrid(Pointi gridPos) const {
    return pTileTable->tiles[tileIdByGrid(gridPos)].uAttributes & (TILE_WATER | TILE_SHORE);
}

Vec3f OutdoorTerrain::normalByPos(const Vec3f &pos) const {
    Pointi gridPos = WorldPosToGrid(pos);
    if (!contains(pTerrainNormals, gridPos))
        return Vec3f(0, 0, 1);

    int x0 = GridCellToWorldPosX(gridPos.x);
    int y0 = GridCellToWorldPosY(gridPos.y);

    int dx = pos.x - x0;
    int dy = y0 - pos.y;

    assert(dx >= 0);
    assert(dy >= 0);

    if (dy >= dx) {
        return pTerrainNormals[gridPos][1];
    } else {
        return pTerrainNormals[gridPos][0];
    }
}

bool OutdoorTerrain::isSlopeTooHighByPos(const Vec3f &pos) const {
    Pointi gridPos = WorldPosToGrid(pos);

    TileGeometry tile = tileGeometryByGrid(gridPos);

    int dx = pos.x - tile.x0;
    int dy = tile.y0 - pos.y;

    assert(dx >= 0);
    assert(dy >= 0);

    int z1, z2, z3;
    if (dy >= dx) {
        //  lower-left triangle
        //  z3 | \
        //     |   \
        //     |     \
        //     |______ \
        //  z1           z2
        z1 = tile.z01;
        z2 = tile.z11;
        z3 = tile.z00;
    } else {
        // upper-right triangle
        //  z2_______ z1
        //    \     |
        //      \   |
        //        \ |
        //          z3
        z1 = tile.z10;
        z2 = tile.z00;
        z3 = tile.z11;
    }

    int yMin = std::min({z1, z2, z3});
    int yMax = std::max({z1, z2, z3});
    return yMax - yMin > 512;
}

void reconstruct(const OutdoorLocation_MM7 &src, OutdoorTerrain *dst) {
    std::array<int, 4> baseTileIds;
    for (int i = 0; i < 4; i++) {
        dst->pTileTypes[i] = static_cast<Tileset>(src.tileTypes[i].tileset);
        baseTileIds[i] = pTileTable->tileId(dst->pTileTypes[i], TILE_VARIANT_BASE1);
    }

    for (int y = 0; y < 128; y++)
        for (int x = 0; x < 128; x++)
            dst->pHeightmap[y][x] = src.heightMap[y * 128 + x];

    for (int y = 0; y < 127; y++)
        for (int x = 0; x < 127; x++)
            dst->pTilemap[y][x] = mapToGlobalTileId(baseTileIds, src.tileMap[y * 128 + x]);

    dst->recalculateNormals();
}

void OutdoorTerrain::recalculateNormals() {
    for (int y = 0; y < pTerrainNormals.height(); y++) {
        for (int x = 0; x < pTerrainNormals.width(); x++) {
            TileGeometry tile = tileGeometryByGrid({x, y});

            Vec3f a2 = Vec3f(tile.x1, tile.y1, tile.z11) - Vec3f(tile.x0, tile.y1, tile.z01);
            Vec3f a1 = Vec3f(tile.x0, tile.y0, tile.z00) - Vec3f(tile.x0, tile.y1, tile.z01);
            Vec3f b2 = Vec3f(tile.x0, tile.y0, tile.z00) - Vec3f(tile.x1, tile.y0, tile.z10);
            Vec3f b1 = Vec3f(tile.x1, tile.y1, tile.z11) - Vec3f(tile.x1, tile.y0, tile.z10);

            // TODO(captainurist): use normalize() & retrace.

            Vec3f an = cross(a2, a1);
            float amag = an.length();
            an /= amag;

            Vec3f bn = cross(b2, b1);
            float bmag = bn.length();
            bn /= bmag;

            assert(an.z > 0);
            assert(bn.z > 0);

            pTerrainNormals[y][x][0] = bn;
            pTerrainNormals[y][x][1] = an;
        }
    }
}

OutdoorTerrain::TileGeometry OutdoorTerrain::tileGeometryByGrid(Pointi gridPos) const {
    TileGeometry result;
    result.x0 = GridCellToWorldPosX(gridPos.x);
    result.y0 = GridCellToWorldPosY(gridPos.y);
    result.x1 = GridCellToWorldPosX(gridPos.x + 1);
    result.y1 = GridCellToWorldPosY(gridPos.y + 1);
    result.z00 = heightByGrid(gridPos);
    result.z01 = heightByGrid(gridPos + Pointi(0, 1));
    result.z10 = heightByGrid(gridPos + Pointi(1, 0));
    result.z11 = heightByGrid(gridPos + Pointi(1, 1));
    return result;
}
