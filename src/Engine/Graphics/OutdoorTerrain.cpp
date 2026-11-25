#include "OutdoorTerrain.h"

#include <array>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <utility>

#include "Engine/Tables/TileTable.h"
#include "Engine/Seasons.h"

#include "Outdoor.h"
#include "Engine/Data/TileEnumFunctions.h"
#include "Library/Logger/Logger.h"

OutdoorTerrain::OutdoorTerrain() {
    // Map is 127x127 squares.
    _heightMap = Image<uint8_t>::solid(0, 128, 128);
    _tileMap = Image<int16_t>::solid(0, 127, 127);
    _originalTileMap = Image<int16_t>::solid(0, 127, 127);
    _normalMap = Image<std::array<Vec3f, 2>>::solid({Vec3f(0, 0, 1), Vec3f(0, 0, 1)}, 127, 127);
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

float OutdoorTerrain::heightByGrid(Pointi gridPos) const {
    if (!_heightMap.geometry().contains(gridPos))
        return 0;

    return 32 * _heightMap[gridPos];
}

float OutdoorTerrain::heightByPos(const Vec3f &pos) const {
    // TODO(captainurist): this function had some code that would push the party -60 units down when on a water tile AND
    //                     not water-walking, but this isn't enabled in the game. I tried it, and it actually looks
    //                     good, as if the party is actually a bit submerged and swimming. The only problem is that
    //                     party would be jerked up upon coming ashore, and this just looks ugly. Find a way to
    //                     reimplement this properly.
    if (!std::isfinite(pos.x) || !std::isfinite(pos.y)) {
        // `worldToGrid` below would `static_cast<int>` a NaN/Inf float, which is undefined behavior.
        MM_WARNING("OutdoorTerrain::heightByPos: non-finite pos ({}, {}), returning 0.", pos.x, pos.y);
        return 0;
    }

    Pointi gridPos = worldToGrid(pos);
    TileGeometry tile = tileGeometryByGrid(gridPos);
    if (tile.z00 != tile.z10 || tile.z10 != tile.z11 || tile.z11 != tile.z01) {
        // On a slope.
        int originz;
        int lz;
        int rz;
        float rpos;
        float lpos;
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

        // `lpos`/`rpos` are in `[0, 512)` for every finite `pos` that `worldToGrid` maps into `gridPos`, so a value
        // outside that range here means `pos` is inconsistent with the tile `worldToGrid` picked for it.
        if (lpos < 0 || lpos >= 512 || rpos < 0 || rpos >= 512) {
            MM_WARNING("OutdoorTerrain::heightByPos: pos ({}, {}) out of range for its own tile "
                                 "gridPos ({}, {}) (lpos={}, rpos={}), clamping.",
                                 pos.x, pos.y, gridPos.x, gridPos.y, lpos, rpos);
            lpos = std::clamp(lpos, 0.0f, 511.0f);
            rpos = std::clamp(rpos, 0.0f, 511.0f);
        }

        return originz + rpos * (rz - originz) / 512 + lpos * (lz - originz) / 512;
    } else {
        // On flat terrain.
        return tile.z00;
    }
}

int OutdoorTerrain::tileIdByGrid(Pointi gridPos) const {
    if (!_tileMap.geometry().contains(gridPos))
        return 0;

    return _tileMap[gridPos];
}

const TileData &OutdoorTerrain::tileDataByGrid(Pointi gridPos) const {
    return pTileTable->tile(tileIdByGrid(gridPos));
}

Tileset OutdoorTerrain::tilesetByGrid(Pointi gridPos) const {
    if (!_tileMap.geometry().contains(gridPos))
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
    if (!_normalMap.geometry().contains(gridPos))
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
            Pointi pos(x, y);
            Tileset tileset = pTileTable->tile(map[y][x]).tileset;

            if (tileset == TILESET_INVALID) {
                // Some MM8 outdoor maps encode a handful of terrain cells (typically under
                // decorations) with a reserved/unused local tile id that resolves to the TileTable's
                // global "pending" placeholder entry instead of a real terrain tile - rendering as a
                // visible missing-texture tile in a spot the original game never shows as broken.
                // Adopt whichever real tileset the majority of the 8 neighbors use instead.
                std::array<int, 9> neighborTilesetCounts{}; // Indexed by Tileset's underlying value.
                for (Direction dir : allDirections()) {
                    Tileset neighborTileset = snappedTilesetByGrid(pos + offsetForDirection(dir));
                    if (neighborTileset != TILESET_INVALID)
                        neighborTilesetCounts[std::to_underlying(neighborTileset)]++;
                }

                auto bestIt = std::max_element(neighborTilesetCounts.begin(), neighborTilesetCounts.end());
                if (*bestIt == 0)
                    continue; // Surrounded by other invalid cells too - nothing sensible to fall back to.
                tileset = static_cast<Tileset>(bestIt - neighborTilesetCounts.begin());

                int baseTileId = pTileTable->tileId(tileset, isRoad(tileset) ? TILE_VARIANT_ROAD_N_S_E_W : TILE_VARIANT_BASE1);
                if (!baseTileId)
                    continue;
                map[y][x] = baseTileId;
            }

            if (isRoad(tileset) || tileset == TILESET_DIRT) // No transitions for roads & dirt.
                continue;

            Directions transitions = 0;
            for (Direction dir : allDirections())
                if (snappedTilesetByGrid(pos + offsetForDirection(dir)) != tileset)
                    transitions |= dir;

            int tileId = pTileTable->tileId(tileset, tileVariantForTransitionDirections(transitions));
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
