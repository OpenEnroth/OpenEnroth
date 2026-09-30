#pragma once

#include <string_view>

#include "Engine/Tables/ResourceMaskTable.h"

#include "Utility/Memory/Blob.h"

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
     * @return                          The icon, masked as `resource_mask_table.json` says, or an empty image if
     *                                  there is no such icon.
     */
    RgbaImage icon(std::string_view filename);

    /**
     * @param filename                  Name of an image in `bitmaps.lod`, case-insensitive.
     * @return                          The bitmap, masked as `resource_mask_table.json` says and desaturated, or an
     *                                  empty image if there is no such bitmap.
     */
    RgbaImage bitmap(std::string_view filename);

 private:
    LodReader _eventsLodReader;
    LodReader _iconsLodReader;
    LodReader _bitmapsLodReader;
    ResourceMaskTable _masks;
};
