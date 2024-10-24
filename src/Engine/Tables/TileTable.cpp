#include "TileTable.h"

#include "Engine/AssetsManager.h"
#include "Engine/Random/Random.h"

TileTable *pTileTable;

//----- (00487E3B) --------------------------------------------------------
TileData *TileTable::GetTileById(unsigned int uTileID) {
    return &tiles[uTileID];
}

//----- (00487ED6) --------------------------------------------------------
int TileTable::GetTileForTerrainType(Tileset terrain_type, bool not_random) {
    int v5;  // edx@3
    int v6;  // edx@11

    if (not_random || terrain_type > Tileset_Tropical) {
        return GetTileId(terrain_type, TILE_SECT_Base1);
    }
    v5 = vrng->random(50);
    if (v5 < 20) {
        return GetTileId(terrain_type, TILE_SECT_Base1);
    } else if (v5 < 30) {
        return GetTileId(terrain_type, TILE_SECT_Base2_NS);
    } else if (v5 < 40) {
        return GetTileId(terrain_type, TILE_SECT_Base3_EW);
    } else if (v5 < 48) {
        return GetTileId(terrain_type, TILE_SECT_Base4_N_E);
    }
    return GetTileId(terrain_type, vrng->randomSample(allSpecialTileSects()));
}

//----- (00487F84) --------------------------------------------------------
unsigned int TileTable::GetTileId(Tileset uTerrainType, TILE_SECT uSection) {
    for (size_t i = 0; i < tiles.size(); ++i) {
        if ((tiles[i].tileset == uTerrainType) &&
            (tiles[i].uSection == uSection))
            return i;
    }
    return 0;
}
