#include "OutdoorTerrain.h"

#include <cassert>
#include <algorithm>

#include "Engine/Tables/TileTable.h"
#include "Engine/Snapshots/CompositeSnapshots.h"
#include "Engine/Snapshots/EntitySnapshots.h"

#include "Library/Snapshots/CommonSnapshots.h"

#include "Outdoor.h"

int static mapToGlobalTileId(const std::array<int, 4> &baseIds, int localTileId) {
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

    int tileSetIndex = (localTileId - 90) / 36;
    int tileSetOffset = (localTileId - 90) % 36;
    return baseIds[tileSetIndex] + tileSetOffset;
}

OutdoorTerrain::OutdoorTerrain() {
    pHeightmap = Image<uint8_t>::solid(128, 128, 0);
    pTilemap = Image<int16_t>::solid(128, 128, 0);
    pTerrainNormals = Image<std::array<Vec3f, 2>>::solid(128, 128, {Vec3f(0, 0, 1), Vec3f(0, 0, 1)});
}

void OutdoorTerrain::CreateDebugTerrain() {
    int tileId = pTileTable->tileId(TILE_SET_GRASS, TILE_VARIANT_BASE1);

    pHeightmap.fill(0);
    pTilemap.fill(tileId);
    pTerrainNormals.fill({Vec3f(0, 0, 1), Vec3f(0, 0, 1)});

    pTileTypes[0] = TILE_SET_GRASS;
    pTileTypes[1] = TILE_SET_WATER;
    pTileTypes[2] = TILE_SET_BADLANDS;
    pTileTypes[3] = TILE_SET_ROAD_GRASS_COBBLE;
}

int OutdoorTerrain::heightByGrid(Vec2i gridPos) const {
    if (gridPos.x < 0 || gridPos.x > 127 || gridPos.y < 0 || gridPos.y > 127)
        return 0;

    return 32 * pHeightmap[gridPos.y][gridPos.x];
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

    Vec2i gridPos = WorldPosToGrid(pos);

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

int OutdoorTerrain::tileIdByGrid(Vec2i gridPos) const {
    if (gridPos.x < 0 || gridPos.x > 127 || gridPos.y < 0 || gridPos.y > 127)
        return 0;

    return pTilemap[gridPos.y][gridPos.x];
}

TileSet OutdoorTerrain::tileSetByGrid(Vec2i gridPos) const {
    if (gridPos.x < 0 || gridPos.x > 127 || gridPos.y < 0 || gridPos.y > 127)
        return TILE_SET_INVALID;

    return pTileTable->tiles[pTilemap[gridPos.y][gridPos.x]].tileset;
}

TileSet OutdoorTerrain::tileSetByPos(const Vec3f &pos) const {
    return tileSetByGrid(WorldPosToGrid(pos));
}

bool OutdoorTerrain::isWaterByGrid(Vec2i gridPos) const {
    return pTileTable->tiles[tileIdByGrid(gridPos)].uAttributes & TILE_WATER;
}

bool OutdoorTerrain::isWaterOrShoreByGrid(Vec2i gridPos) const {
    return pTileTable->tiles[tileIdByGrid(gridPos)].uAttributes & (TILE_WATER | TILE_SHORE);
}

bool OutdoorTerrain::isWaterByPos(const Vec3f &pos) const {
    return isWaterByGrid(WorldPosToGrid(pos));
}

Vec3f OutdoorTerrain::normalByPos(const Vec3f &pos) const {
    Vec2i gridPos = WorldPosToGrid(pos);

    int x0 = GridCellToWorldPosX(gridPos.x);
    int y0 = GridCellToWorldPosY(gridPos.y);

    int dx = pos.x - x0;
    int dy = y0 - pos.y;

    assert(dx >= 0);
    assert(dy >= 0);

    if (dy >= dx) {
        return pTerrainNormals[gridPos.y][gridPos.x][1];
    } else {
        return pTerrainNormals[gridPos.y][gridPos.x][0];
    }
}

bool OutdoorTerrain::isSlopeTooHighByPos(const Vec3f &pos) const {
    Vec2i gridPos = WorldPosToGrid(pos);

    TileGeometry tile = tileGeometryByGrid(gridPos);

    int dx = std::abs(pos.x - tile.x0), dz = std::abs(tile.y0 - pos.y);

    int y1, y2, y3;
    if (dz >= dx) {
        y1 = tile.z01;
        y2 = tile.z11;
        y3 = tile.z00;
        //  lower-left triangle
        //  y3 | \
        //     |   \
        //     |     \
        //     |______ \
        //  y1           y2
    } else {
        y1 = tile.z10;
        y2 = tile.z00;
        y3 = tile.z11;

        // upper-right
        //  y2_______ y1
        //    \     |
        //      \   |
        //        \ |
        //          y3
    }

    int y_min = std::min(y1, std::min(y2, y3));  // не верно при подъёме на склон
    int y_max = std::max(y1, std::max(y2, y3));
    return (y_max - y_min) > 512;
}

void reconstruct(const OutdoorLocation_MM7 &src, OutdoorTerrain *dst) {
    std::array<int, 4> baseTileIds;
    for (int i = 0; i < 4; i++) {
        dst->pTileTypes[i] = static_cast<TileSet>(src.tileTypes[i].tileset);
        baseTileIds[i] = pTileTable->tileId(dst->pTileTypes[i], TILE_VARIANT_BASE1);
    }

    for (int y = 0; y < 128; y++) {
        for (int x = 0; x < 128; x++) {
            dst->pHeightmap[y][x] = src.heightMap[y * 128 + x];
            dst->pTilemap[y][x] = mapToGlobalTileId(baseTileIds, src.tileMap[y * 128 + x]);
        }
    }

    dst->recalculateNormals();
}

void OutdoorTerrain::recalculateNormals() {
    for (int y = 0; y < 128; y++) {
        for (int x = 0; x < 128; x++) {
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

OutdoorTerrain::TileGeometry OutdoorTerrain::tileGeometryByGrid(Vec2i gridPos) const {
    TileGeometry result;
    result.x0 = GridCellToWorldPosX(gridPos.x);
    result.y0 = GridCellToWorldPosY(gridPos.y);
    result.x1 = GridCellToWorldPosX(gridPos.x + 1);
    result.y1 = GridCellToWorldPosY(gridPos.y + 1);
    result.z00 = heightByGrid(gridPos);
    result.z01 = heightByGrid(gridPos + Vec2i(0, 1));
    result.z10 = heightByGrid(gridPos + Vec2i(1, 0));
    result.z11 = heightByGrid(gridPos + Vec2i(1, 1));
    return result;
}
