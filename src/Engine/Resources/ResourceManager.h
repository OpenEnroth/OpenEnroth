#pragma once

#include <optional>
#include <string_view>

#include "Engine/Tables/ResourceMaskTable.h"

#include "Utility/Memory/Blob.h"

#include "Library/Geometry/Size.h"
#include "Library/Image/Image.h"
#include "Library/Lod/LodReader.h"

/**
 * This class provides access to everything in `/data` folder.
 */
class ResourceManager {
 public:
    ResourceManager();
    ~ResourceManager();

    void open();

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
     * @param filename                  Name of a sprite in `sprites.lod`, case-insensitive.
     * @return                          The sprite with its palette indices in the red channel, or an empty image if
     *                                  there is no such sprite.
     */
    RgbaImage sprite(std::string_view filename);

    /**
     * @param filename                  Name of a sprite in `sprites.lod`, case-insensitive.
     * @return                          The sprite's size, or `std::nullopt` if there is no such sprite.
     */
    std::optional<Sizei> spriteSize(std::string_view filename);

 private:
    LodReader _eventsLodReader;
    LodReader _iconsLodReader;
    LodReader _bitmapsLodReader;
    LodReader _spritesLodReader;
    ResourceMaskTable _masks;
};
