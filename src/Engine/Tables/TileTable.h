#pragma once

#include <vector>
#include <unordered_map>
#include <utility>

#include "Engine/Data/TileData.h"

#include "Utility/Hash.h"

struct TriBlob;

class TileTable {
 public:
    /**
     * @param tileset                   Tileset to use.
     * @param nonRandom                 Whether to get the base tile id, or a random base or special tile. Looks like
     *                                  this might have been used in the map editor.
     * @return                          Tile id for the requested tileset.
     * @offset 0x00487ED6
     */
    int tileIdForTileset(Tileset tileset, bool nonRandom) const;

    /**
     *
     * @param tileset                   Tileset to use.
     * @param variant                   Tile variant.
     * @return                          Tile id for the given tileset and variant, or `0` if the tile wasn't found.
     *                                  Note that tile with id '0' belongs to `TILESET_INVALID`.
     * @offset 0x00487F84
     */
    int tileId(Tileset tileset, TileVariant variant) const;

    const TileData &tile(int id) const;

    void addTile(TileData tileData);

    friend void deserialize(const TriBlob &src, TileTable *dst); // In TableSerialization.cpp.

 private:
    std::vector<TileData> _tiles; // Tile by id.
    std::unordered_map<std::pair<Tileset, TileVariant>, int> _idByTilesetVariant;
};

extern TileTable *pTileTable;
