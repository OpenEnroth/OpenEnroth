#include "TileTable.h"

#include "Engine/AssetsManager.h"
#include "Engine/Random/Random.h"

TileTable *pTileTable;

//----- (00487ED6) --------------------------------------------------------
int TileTable::tileIdForTileset(Tileset terrain_type, bool nonRandom) {
    int v5;  // edx@3
    int v6;  // edx@11

    if (nonRandom || terrain_type > Tileset_Tropical) {
        return tileId(terrain_type, TILE_SECT_Base1);
    }
    v5 = vrng->random(50);
    if (v5 < 20) {
        return tileId(terrain_type, TILE_SECT_Base1);
    } else if (v5 < 30) {
        return tileId(terrain_type, TILE_SECT_Base2_NS);
    } else if (v5 < 40) {
        return tileId(terrain_type, TILE_SECT_Base3_EW);
    } else if (v5 < 48) {
        return tileId(terrain_type, TILE_SECT_Base4_N_E);
    }
    return tileId(terrain_type, vrng->randomSample(allSpecialTileSects()));
}

//----- (00487F84) --------------------------------------------------------
int TileTable::tileId(Tileset tileset, TILE_SECT section) {
    for (size_t i = 0; i < tiles.size(); ++i) {
        if ((tiles[i].tileset == tileset) &&
            (tiles[i].uSection == section))
            return i;
    }
    return 0;
}
