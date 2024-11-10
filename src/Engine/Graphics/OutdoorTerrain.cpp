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

    if (tile.v00.z != tile.v10.z || tile.v10.z != tile.v11.z || tile.v11.z != tile.v01.z) {
        // On a slope.
        if (std::abs(tile.v00.y - pos.y) >= std::abs(pos.x - tile.v00.x)) {
            originz = tile.v01.z;
            lz = tile.v11.z;
            rz = tile.v00.z;
            lpos = pos.x - tile.v00.x;
            rpos = pos.y - tile.v11.y;
        } else {
            originz = tile.v10.z;
            lz = tile.v00.z;
            rz = tile.v11.z;
            lpos = tile.v11.x - pos.x;
            rpos = tile.v00.y - pos.y;
        }

        assert(lpos >= 0 && lpos < 512);
        assert(rpos >= 0 && rpos < 512);

        // (x >> 9) is basically (x / 512) but with consistent rounding towards -inf.
        return originz + ((rpos * (rz - originz)) >> 9) + ((lpos * (lz - originz)) >> 9);
    } else {
        // On flat terrain.
        return tile.v00.z;
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

SoundId OutdoorTerrain::soundIdByGrid(Vec2i gridPos, bool isRunning) const {
    // TODO(captainurist): this doesn't take seasons into account.
    switch (tileSetByGrid(gridPos)) {
    case TILE_SET_GRASS:
        return isRunning ? SOUND_RunGrass : SOUND_WalkGrass;
    case TILE_SET_SNOW:
        return isRunning ? SOUND_RunSnow : SOUND_WalkSnow;
    case TILE_SET_DESERT:
        return isRunning ? SOUND_RunDesert : SOUND_WalkDesert;
    case TILE_SET_COOLED_LAVA:
        return isRunning ? SOUND_RunCooledLava : SOUND_WalkCooledLava;
    case TILE_SET_INVALID: // Use dirt sounds for invalid tiles.
    case TILE_SET_DIRT:
        // Water sounds were used
        return isRunning ? SOUND_RunDirt : SOUND_WalkDirt;
    case TILE_SET_WATER:
        // Dirt sounds were used
        return isRunning ? SOUND_RunWater : SOUND_WalkWater;
    case TILE_SET_BADLANDS:
        return isRunning ? SOUND_RunBadlands : SOUND_WalkBadlands;
    case TILE_SET_SWAMP:
        return isRunning ? SOUND_RunSwamp : SOUND_WalkSwamp;
    case TILE_SET_TROPICAL:
        // TODO(Nik-RE-dev): is that correct?
        return isRunning ? SOUND_RunGrass : SOUND_WalkGrass;
    case TILE_SET_ROAD_GRASS_COBBLE:
    case TILE_SET_ROAD_GRASS_DIRT:
    case TILE_SET_ROAD_SNOW_COBBLE:
    case TILE_SET_ROAD_SNOW_DIRT:
    case TILE_SET_ROAD_SAND_COBBLE:
    case TILE_SET_ROAD_SAND_DIRT:
    case TILE_SET_ROAD_VOLCANO_COBBLE:
    case TILE_SET_ROAD_VOLCANO_DIRT:
    case TILE_SET_ROAD_CRACKED_COBBLE:
    case TILE_SET_ROAD_CRACKED_DIRT:
    case TILE_SET_ROAD_SWAMP_COBBLE:
    case TILE_SET_ROAD_SWAMP_DIRT:
    case TILE_SET_ROAD_TROPICAL_COBBLE:
    case TILE_SET_ROAD_TROPICAL_DIRT:
        return isRunning ? SOUND_RunRoad : SOUND_WalkRoad;
    case TILE_SET_CITY:
    case TILE_SET_ROAD_CITY_STONE:
        // TODO(Nik-RE-dev): is that correct?
    default:
        return isRunning ? SOUND_RunGround : SOUND_WalkGround;
    }
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

    TileGeometry tile = tileGeometryByGrid(gridPos);

    Vec3f side1, side2;

    int dx = std::abs(pos.x - tile.v00.x);
    int dy = std::abs(tile.v00.y - pos.y);
    if (dy >= dx) {
        side2 = tile.v11 - tile.v01;
        side1 = tile.v00 - tile.v01;
        /*       |\
           side1 |  \
                 |____\
                 side 2      */
    } else {
        side2 = tile.v00 - tile.v10;
        side1 = tile.v11 - tile.v10;
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

    int dx = std::abs(pos.x - tile.v00.x), dz = std::abs(tile.v00.y - pos.y);

    int y1, y2, y3;
    if (dz >= dx) {
        y1 = tile.v01.z;
        y2 = tile.v11.z;
        y3 = tile.v00.z;
        //  lower-left triangle
        //  y3 | \
        //     |   \
        //     |     \
        //     |______ \
        //  y1           y2
    } else {
        y1 = tile.v10.z;
        y2 = tile.v00.z;
        y3 = tile.v11.z;

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
    int x0 = GridCellToWorldPosX(gridPos.x);
    int y0 = GridCellToWorldPosY(gridPos.y);
    int x1 = GridCellToWorldPosX(gridPos.x + 1);
    int y1 = GridCellToWorldPosY(gridPos.y + 1);

    int z00 = heightByGrid(gridPos);
    int z01 = heightByGrid(gridPos + Vec2i(0, 1));
    int z10 = heightByGrid(gridPos + Vec2i(1, 0));
    int z11 = heightByGrid(gridPos + Vec2i(1, 1));

    TileGeometry result;
    result.v00 = Vec3f(x0, y0, z00);
    result.v01 = Vec3f(x0, y1, z01);
    result.v10 = Vec3f(x1, y0, z10);
    result.v11 = Vec3f(x1, y1, z11);
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
