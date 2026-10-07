#pragma once

#include <memory>
#include <span>
#include <string_view>

#include "Engine/Tables/ResourceMaskTable.h"

#include "Utility/Memory/Blob.h"

#include "Library/Geometry/Size.h"
#include "Library/Image/Image.h"
#include "Library/Image/Palette.h"
#include "Library/Lod/LodReader.h"
#include "Library/LodFormats/LodFont.h"
#include "Library/LodFormats/LodImage.h"

class TileGenerator;
class TileTable;

/**
 * This class provides access to everything in `/data` folder.
 */
class ResourceManager {
 public:
    ResourceManager();
    ~ResourceManager();

    /**
     * Opens the LODs in `data` and loads the mask table.
     *
     * @param saturation                Saturation multiplier for the bitmaps and palettes this class returns.
     * @param lightness                 Lightness multiplier for the bitmaps and palettes this class returns.
     */
    void open(float saturation = 1.0f, float lightness = 1.0f);

    Blob eventsData(std::string_view filename);

    /**
     * @param filename                  Name of an image in `icons.lod`, case-insensitive.
     * @return                          The icon, or an empty image if there is no such icon.
     */
    RgbaImage icon(std::string_view filename);

    /**
     * @param filename                  Name of an image in `bitmaps.lod`, case-insensitive.
     * @return                          The desaturated bitmap, or an empty image if there is no such bitmap.
     */
    RgbaImage bitmap(std::string_view filename);

    /**
     * @param filename                  Name of a generated transition tile, as `addGeneratedTiles()` put it into the
     *                                  tile table. The tile is generated on first use.
     * @return                          The desaturated tile.
     */
    RgbaImage generated(std::string_view filename);

    // TODO(captainurist): maybe return the generated tile data and let the caller add it to the table?
    /**
     * Adds the generated transition tiles to the tile table. Their images are only generated once `generated()` asks
     * for them.
     *
     * @param table                     Tile table to add the generated tiles to.
     */
    void addGeneratedTiles(TileTable *table);

    /**
     * @param filename                  Name of an image in `icons.lod`, case-insensitive.
     * @return                          The icon as stored in the LOD, unmasked, or an empty image if there is no
     *                                  such icon.
     */
    LodImage rawIcon(std::string_view filename);

    /**
     * @param filename                  Name of an image in `bitmaps.lod`, case-insensitive.
     * @return                          The bitmap as stored in the LOD, unmasked and not desaturated, or an empty
     *                                  image if there is no such bitmap.
     */
    LodImage rawBitmap(std::string_view filename);

    /**
     * @param filename                  Name of a font in `icons.lod`, case-insensitive.
     * @return                          The font.
     * @throws Exception                If there is no such font.
     */
    LodFont font(std::string_view filename);

    /**
     * @param paletteId                 Palette id, the `NNN` in the `palNNN` image in `bitmaps.lod`.
     * @return                          The palette adjusted per `open()`, or a grayscale one if `bitmaps.lod` has no
     *                                  such palette.
     *                                  Most ids are unused, so a missing palette isn't logged.
     */
    Palette palette(int paletteId);

    /**
     * @param filename                  Name of a sprite in `sprites.lod`, case-insensitive.
     * @return                          The sprite with its palette indices in the red channel, or an empty image if
     *                                  there is no such sprite.
     */
    RgbaImage sprite(std::string_view filename);

    /**
     * @param filename                  Name of a sprite in `sprites.lod`, case-insensitive.
     * @return                          The sprite's size, or an empty size if there is no such sprite.
     */
    Sizei spriteSize(std::string_view filename);

 private:
    /**
     * Applies the saturation and lightness passed to `open()`.
     *
     * @param colors                    Colors to adjust in place.
     */
    void desaturate(std::span<Color> colors) const;

 private:
    LodReader _eventsLodReader;
    LodReader _iconsLodReader;
    LodReader _bitmapsLodReader;
    LodReader _spritesLodReader;
    ResourceMaskTable _masks;
    float _saturation = 1.0f;
    float _lightness = 1.0f;
    std::unique_ptr<TileGenerator> _tileGenerator;
};
