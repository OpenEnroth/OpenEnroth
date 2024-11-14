#include "OutdoorTerrain.h"

#include <algorithm>

#include "Engine/Tables/TileTable.h"

#include "Outdoor.h"

//----- (0047CCE2) --------------------------------------------------------
bool OutdoorTerrain::ZeroLandscape() {
    this->pHeightmap.fill(0);
    this->pTilemap.fill(90);
    this->pAttributemap.fill(0);
    return true;
}

//----- (0047F420) --------------------------------------------------------
void OutdoorTerrain::LoadBaseTileIds() {
    for (unsigned i = 0; i < 3; ++i)
        pTileTypes[i].uTileID = pTileTable->tileIdForTileset(pTileTypes[i].tileset, 1);
}

void OutdoorTerrain::CreateDebugTerrain() {
    ZeroLandscape();
    pTileTypes[0].tileset = TILE_SET_GRASS;
    pTileTypes[1].tileset = TILE_SET_WATER;
    pTileTypes[2].tileset = TILE_SET_BADLANDS;
    pTileTypes[3].tileset = TILE_SET_ROAD_GRASS_COBBLE;
    LoadBaseTileIds();
}

//----- (00488F2E) --------------------------------------------------------
//----- (0047EE16) --------------------------------------------------------
int OutdoorTerrain::heightByGrid(Vec2i gridPos) const {
    if (gridPos.x < 0 || gridPos.x > 127 || gridPos.y < 0 || gridPos.y > 127)
        return 0;

    return 32 * pHeightmap[gridPos.y * 128 + gridPos.x];
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

        assert(lpos >= 0 && lpos < 512);
        assert(rpos >= 0 && rpos < 512);

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

    return mapToGlobalTileId(pTilemap[gridPos.y * 128 + gridPos.x]);
}

TileSet OutdoorTerrain::tileSetByGrid(Vec2i gridPos) const {
    if (gridPos.x < 0 || gridPos.x > 127 || gridPos.y < 0 || gridPos.y > 127)
        return TILE_SET_INVALID;

    int localTileId = pTilemap[gridPos.y * 128 + gridPos.x];

    if (localTileId >= 1 && localTileId <= 12)
        return TILE_SET_DIRT; // See comment in mapToGlobalTileId.

    if (localTileId >= 234 || localTileId < 90)
        return TILE_SET_INVALID;

    int tileSetIndex = (localTileId - 90) / 36;
    return pTileTypes[tileSetIndex].tileset;
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

    // TODO(captainurist): why is this not returning a normal from pTerrainNormals?

    TileGeometry tile = tileGeometryByGrid(gridPos);

    Vec3f side1, side2;

    int dx = std::abs(pos.x - tile.x0);
    int dy = std::abs(tile.y0 - pos.y);
    if (dy >= dx) {
        side2 = Vec3f(tile.x1, tile.y1, tile.z11) - Vec3f(tile.x0, tile.y1, tile.z01);
        side1 = Vec3f(tile.x0, tile.y0, tile.z00) - Vec3f(tile.x0, tile.y1, tile.z01);
        /*       |\
           side1 |  \
                 |____\
                 side 2      */
    } else {
        side2 = Vec3f(tile.x0, tile.y0, tile.z00) - Vec3f(tile.x1, tile.y0, tile.z10);
        side1 = Vec3f(tile.x1, tile.y1, tile.z11) - Vec3f(tile.x1, tile.y0, tile.z10);
        /*   side 2
             _____
             \    |
               \  | side 1
                 \|       */
    }

    Vec3f n = cross(side2, side1);
    float mag = n.length();
    if (fabsf(mag) < 1e-6f) {
        return Vec3f(0, 0, 1);
    } else {
        return n / mag;
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

int OutdoorTerrain::mapToGlobalTileId(int localTileId) const {
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
    return pTileTypes[tileSetIndex].uTileID + tileSetOffset;
}
