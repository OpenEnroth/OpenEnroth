#pragma once

#include <unordered_map>
#include <vector>
#include <utility>

#include "Engine/Data/TileEnums.h"

#include "Library/Image/Image.h"

#include "Utility/Hash.h"

class TileGenerator {
 public:
    TileGenerator();
    ~TileGenerator();

    void generateTiles(bool force = false);

 private:
    RgbaImage generateTile(Tileset tileset, TileVariant variant);
    RgbaImageView loadTile(Tileset tileset, TileVariant variant);
    TileVariant findSpanningVariant(Directions currentDirections, Directions targetDirections) const;
    void blendTile(RgbaImageView base, RgbaImageView dirt, RgbaImageView layer1, RgbaImage *layer0) const;

 private:
    std::vector<std::pair<TileVariant, Directions>> _standardTiles;
    std::unordered_map<std::pair<Tileset, TileVariant>, RgbaImage> _tileByTilesetVariant;
};

extern TileGenerator *pTileGenerator;
