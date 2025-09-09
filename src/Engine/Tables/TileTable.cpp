#include "TileTable.h"

#include <utility>

#include "Engine/Random/Random.h"
#include "Engine/Data/TileEnumFunctions.h"

#include "Utility/MapAccess.h"

TileTable *pTileTable;

int TileTable::tileIdForTileset(Tileset terrain_type, bool nonRandom) const {
    if (nonRandom || isRoad(terrain_type))
        return tileId(terrain_type, isRoad(terrain_type) ? TILE_VARIANT_ROAD_N_S_E_W : TILE_VARIANT_BASE1);

    int v5 = vrng->random(50);
    if (v5 < 20) {
        return tileId(terrain_type, TILE_VARIANT_BASE1);
    } else if (v5 < 30) {
        return tileId(terrain_type, TILE_VARIANT_BASE2);
    } else if (v5 < 40) {
        return tileId(terrain_type, TILE_VARIANT_BASE3);
    } else if (v5 < 48) {
        return tileId(terrain_type, TILE_VARIANT_BASE4);
    } else {
        return tileId(terrain_type, vrng->randomSample(allSpecialTileVariants()));
    }
}

int TileTable::tileId(Tileset tileset, TileVariant section) const {
    return valueOr(_idByTilesetVariant, std::pair(tileset, section), 0);
}

const TileData &TileTable::tile(int id) const {
    return _tiles[id];
}

void TileTable::addTile(TileData tileData) {
    std::pair key(tileData.tileset, tileData.variant);
    assert(!_idByTilesetVariant.contains(key));

    _idByTilesetVariant.emplace(key, _tiles.size());
    _tiles.push_back(std::move(tileData));
}
