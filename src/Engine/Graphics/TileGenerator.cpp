#include "TileGenerator.h"

#include <cassert>
#include <utility>

#include "Engine/AssetsManager.h"
#include "Engine/EngineFileSystem.h"
#include "Engine/LodTextureCache.h"
#include "Engine/Data/TileEnumFunctions.h"
#include "Engine/Tables/TileTable.h"
#include "Engine/Graphics/Image.h"
#include "Library/Image/ImageFunctions.h"
#include "Library/Image/Png.h"
#include "Library/LodFormats/LodImage.h"

#include "Library/Serialization/Serialization.h"

#include "Utility/MapAccess.h"

TileGenerator *pTileGenerator = nullptr;

TileGenerator::TileGenerator() {
    for (TileVariant variant : allTransitionTileVariants())
        if (!allGeneratedTileVariants().contains(variant))
            _standardTiles.emplace_back(variant, transitionDirectionsForTileVariant(variant));
}

TileGenerator::~TileGenerator() = default;

void TileGenerator::generateTiles(bool force) {
    for (Tileset tileset : allTerrainTilesets()) {
        if (tileset == TILESET_DIRT || tileset == TILESET_WATER)
            continue; // We don't generate tiles for dirt and water tilesets. For dirt, it makes no sense, and for water we have a skill issue.

        for (TileVariant variant : allGeneratedTileVariants()) {
            TileData tileData;
            tileData.name = fmt::format("generated/tiles/{}_{}.png", toString(tileset), toString(variant));
            tileData.tileset = tileset;
            tileData.variant = variant;
            tileData.flags = TILE_TRANSITION | TILE_GENERATED_TRANSITION;

            if (!ufs->exists(tileData.name) || force)
                ufs->write(tileData.name, png::encode(generateTile(tileset, variant)));

            pTileTable->addTile(std::move(tileData));
        }
    }
}

RgbaImage TileGenerator::generateTile(Tileset tileset, TileVariant variant) {
    assert(allGeneratedTileVariants().contains(variant));

    Directions currentDirections = 0;
    Directions targetDirections = transitionDirectionsForTileVariant(variant);

    RgbaImageView base = loadTile(tileset, TILE_VARIANT_BASE1);
    RgbaImageView dirt = loadTile(TILESET_DIRT, TILE_VARIANT_BASE1);
    RgbaImage result;

    while (currentDirections != targetDirections) {
        TileVariant spanningVariant = findSpanningVariant(currentDirections, targetDirections);
        RgbaImageView layer = loadTile(tileset, spanningVariant);

        if (!result) {
            result = RgbaImage::copy(layer.width(), layer.height(), layer.pixels().data());
        } else {
            blendTile(base, dirt, layer, &result);
        }

        currentDirections |= transitionDirectionsForTileVariant(spanningVariant);
    }

    return result;
}

RgbaImageView TileGenerator::loadTile(Tileset tileset, TileVariant variant) {
    std::pair key(tileset, variant);
    if (const RgbaImage *result = valuePtr(_tileByTilesetVariant, key))
        return *result;

    // Need to load directly from LOD b/c AssetsManager hold saturation-adjusted images.
    LodImage *image = pBitmaps_LOD->loadTexture(pTileTable->tile(pTileTable->tileId(tileset, variant)).name);
    return _tileByTilesetVariant.emplace(key, makeRgbaImage(image->image, image->palette)).first->second;
}

TileVariant TileGenerator::findSpanningVariant(Directions currentDirections, Directions targetDirections) const {
    TileVariant result = TILE_VARIANT_INVALID;
    int maxPopCount = 0;
    for (const auto [variant, directions] : _standardTiles) {
        if (currentDirections & directions)
            continue; // Has common bits with what we already have, we don't need that.

        if (directions & ~targetDirections)
            continue; // Adds bits that we don't need.

        int popCount = std::popcount(static_cast<unsigned int>(std::to_underlying(currentDirections | directions)));
        if (popCount > maxPopCount) {
            result = variant;
            maxPopCount = popCount;
        }
    }

    return maxPopCount == 0 ? TILE_VARIANT_INVALID : result;
}

void TileGenerator::blendTile(RgbaImageView base, RgbaImageView dirt, RgbaImageView layer1, RgbaImage *layer0) const {
    auto dist = [](const Color &l, const Color &r) {
        return std::abs(l.r - r.r) + std::abs(l.g - r.g) + std::abs(l.b - r.b);
    };

    std::span<const Color> basePixels = base.pixels();
    std::span<const Color> dirtPixels = dirt.pixels();
    std::span<const Color> layer1Pixels = layer1.pixels();
    std::span<Color> layer0Pixels = layer0->pixels();

    const int tolerance = 30;

    for (int xy = 0; xy < basePixels.size(); xy++) {
        int dirt0 = dist(layer0Pixels[xy], dirtPixels[xy]);
        int dirt1 = dist(layer1Pixels[xy], dirtPixels[xy]);
        bool isDirt0 = dirt0 < tolerance;
        bool isDirt1 = dirt1 < tolerance;
        if (isDirt0 && isDirt1) {
            if (dirt1 < dirt0)
                layer0Pixels[xy] = layer1Pixels[xy];
            continue;
        } else if (isDirt0) {
            continue;
        } else if (isDirt1) {
            layer0Pixels[xy] = layer1Pixels[xy];
        }

        int transition0 = dist(layer0Pixels[xy], basePixels[xy]);
        int transition1 = dist(layer1Pixels[xy], basePixels[xy]);
        bool isTransition0 = transition0 >= tolerance;
        bool isTransition1 = transition1 >= tolerance;
        if (isTransition0 && isTransition1) {
            if (transition1 > transition0)
                layer0Pixels[xy] = layer1Pixels[xy];
        } else if (isTransition0) {
            // Do nothing.
        } else if (isTransition1) {
            layer0Pixels[xy] = layer1Pixels[xy];
        } else {
            // Do nothing.
        }
    }
}
