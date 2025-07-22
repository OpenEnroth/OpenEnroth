#include "OutdoorTerrain.h"

#include <cassert>
#include <algorithm>

#include "Engine/Tables/TileTable.h"
#include "Engine/Seasons.h"

#include "Outdoor.h"
#include "Engine/Data/TileEnumFunctions.h"

OutdoorTerrain::OutdoorTerrain() {
    // Map is 127x127 squares.
    _heightMap = Image<uint8_t>::solid(128, 128, 0);
    _tileMap = Image<int16_t>::solid(127, 127, 0);
    _originalTileMap = Image<int16_t>::solid(127, 127, 0);
    _normalMap = Image<std::array<Vec3f, 2>>::solid(127, 127, {Vec3f(0, 0, 1), Vec3f(0, 0, 1)});
}

void OutdoorTerrain::createDebugTerrain() {
    int tileId = pTileTable->tileId(TILESET_GRASS, TILE_VARIANT_BASE1);

    _heightMap.fill(0);
    _tileMap.fill(tileId);
    _normalMap.fill({Vec3f(0, 0, 1), Vec3f(0, 0, 1)});

    _tilesets[0] = TILESET_GRASS;
    _tilesets[1] = TILESET_WATER;
    _tilesets[2] = TILESET_BADLANDS;
    _tilesets[3] = TILESET_COBBLE_ROAD;
}

void OutdoorTerrain::changeSeason(int month) {
    assert(month >= 0 && month <= 11);
    std::ranges::transform(_originalTileMap.pixels(), _tileMap.pixels().begin(), [&] (int tileId) {
        const TileData &tileData = pTileTable->tile(tileId);
        Tileset tileset = tilesetForSeason(tileData.tileset, month);
        TileVariant variant = tileset == TILESET_DIRT ? TILE_VARIANT_BASE1 : tileData.variant;
        return pTileTable->tileId(tileset, variant);
    });

    // The call below is needed b/c in winter some snow->dirt->grass transitions turn into just snow.
    recalculateTransitions(&_tileMap);
}

int OutdoorTerrain::heightByGrid(Pointi gridPos) const {
    if (!_heightMap.rect().contains(gridPos))
        return 0;

    return 32 * _heightMap[gridPos];
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

    Pointi gridPos = worldToGrid(pos);

    TileGeometry tile = tileGeometryByGrid(gridPos);

    if (tile.z00 != tile.z10 || tile.z10 != tile.z11 || tile.z11 != tile.z01) {
        // On a slope.
        if (std::abs(tile.v0.y - pos.y) >= std::abs(pos.x - tile.v0.x)) {
            originz = tile.z01;
            lz = tile.z11;
            rz = tile.z00;
            lpos = pos.x - tile.v0.x;
            rpos = pos.y - tile.v1.y;
        } else {
            originz = tile.z10;
            lz = tile.z00;
            rz = tile.z11;
            lpos = tile.v1.x - pos.x;
            rpos = tile.v0.y - pos.y;
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
    if (!_tileMap.rect().contains(gridPos))
        return 0;

    return _tileMap[gridPos];
}

const TileData &OutdoorTerrain::tileDataByGrid(Pointi gridPos) const {
    return pTileTable->tile(tileIdByGrid(gridPos));
}

Tileset OutdoorTerrain::tilesetByGrid(Pointi gridPos) const {
    if (!_tileMap.rect().contains(gridPos))
        return TILESET_INVALID;

    return pTileTable->tile(_tileMap[gridPos]).tileset;
}

Tileset OutdoorTerrain::tilesetByPos(const Vec3f &pos) const {
    return tilesetByGrid(worldToGrid(pos));
}

bool OutdoorTerrain::isWaterByGrid(Pointi gridPos) const {
    return pTileTable->tile(tileIdByGrid(gridPos)).flags & TILE_WATER;
}

bool OutdoorTerrain::isWaterByPos(const Vec3f &pos) const {
    return isWaterByGrid(worldToGrid(pos));
}

bool OutdoorTerrain::isWaterOrShoreByGrid(Pointi gridPos) const {
    return pTileTable->tile(tileIdByGrid(gridPos)).flags & (TILE_WATER | TILE_SHORE);
}

Vec3f OutdoorTerrain::normalByPos(const Vec3f &pos) const {
    Pointi gridPos = worldToGrid(pos);
    if (!_normalMap.rect().contains(gridPos))
        return Vec3f(0, 0, 1);

    Vec2i o = gridToWorld(gridPos);
    int dx = pos.x - o.x;
    int dy = o.y - pos.y;

    assert(dx >= 0);
    assert(dy >= 0);

    if (dy >= dx) {
        return _normalMap[gridPos][1];
    } else {
        return _normalMap[gridPos][0];
    }
}

bool OutdoorTerrain::isSlopeTooHighByPos(const Vec3f &pos) const {
    Pointi gridPos = worldToGrid(pos);

    TileGeometry tile = tileGeometryByGrid(gridPos);

    int dx = pos.x - tile.v0.x;
    int dy = tile.v0.y - pos.y;

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

void OutdoorTerrain::recalculateNormals() {
    for (int y = 0; y < _normalMap.height(); y++) {
        for (int x = 0; x < _normalMap.width(); x++) {
            TileGeometry tile = tileGeometryByGrid({x, y});

            Vec3f a2 = Vec3f(tile.v1.x, tile.v1.y, tile.z11) - Vec3f(tile.v0.x, tile.v1.y, tile.z01);
            Vec3f a1 = Vec3f(tile.v0.x, tile.v0.y, tile.z00) - Vec3f(tile.v0.x, tile.v1.y, tile.z01);
            Vec3f b2 = Vec3f(tile.v0.x, tile.v0.y, tile.z00) - Vec3f(tile.v1.x, tile.v0.y, tile.z10);
            Vec3f b1 = Vec3f(tile.v1.x, tile.v1.y, tile.z11) - Vec3f(tile.v1.x, tile.v0.y, tile.z10);

            Vec3f an = cross(a2, a1);
            an.normalize();

            Vec3f bn = cross(b2, b1);
            bn.normalize();

            assert(an.z > 0);
            assert(bn.z > 0);

            _normalMap[y][x][0] = bn;
            _normalMap[y][x][1] = an;
        }
    }
}

void OutdoorTerrain::recalculateTransitions(Image<int16_t> *tileMap) {
    Image<int16_t> &map = *tileMap;

    auto snappedTilesetByGrid = [&map](Pointi pos) {
        pos.x = std::clamp(pos.x, 0, static_cast<int>(map.width() - 1));
        pos.y = std::clamp(pos.y, 0, static_cast<int>(map.height() - 1));
        return pTileTable->tile(map[pos]).tileset;
    };

    for (int y = 0; y < map.height(); y++) {
        for (int x = 0; x < map.width(); x++) {
            const TileData &tileData = pTileTable->tile(map[y][x]);
            if (isRoad(tileData.tileset) || tileData.tileset == TILESET_DIRT) // No transitions for roads & dirt.
                continue;

            Pointi pos(x, y);
            Directions transitions = 0;
            for (Direction dir : allDirections())
                if (snappedTilesetByGrid(pos + offsetForDirection(dir)) != tileData.tileset)
                    transitions |= dir;

            int tileId = pTileTable->tileId(tileData.tileset, tileVariantForTransitionDirections(transitions));
            if (tileId)
                map[y][x] = tileId;
        }
    }
}

OutdoorTerrain::TileGeometry OutdoorTerrain::tileGeometryByGrid(Pointi gridPos) const {
    TileGeometry result;
    result.v0 = gridToWorld(gridPos);
    result.v1 = gridToWorld(gridPos + Pointi(1, 1));
    result.z00 = heightByGrid(gridPos);
    result.z01 = heightByGrid(gridPos + Pointi(0, 1));
    result.z10 = heightByGrid(gridPos + Pointi(1, 0));
    result.z11 = heightByGrid(gridPos + Pointi(1, 1));
    return result;
}
