#pragma once

#include <string_view>

#include "Engine/Tables/ResourceMaskTable.h"

#include "Utility/Memory/Blob.h"

#include "Library/Image/Image.h"
#include "Library/Lod/LodReader.h"
#include "Library/LodFormats/LodImage.h"

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
     * @param filename                  Name of an image in `icons.lod`, case-insensitive. A missing LOD image is
     *                                  replaced with "pending".
     * @return                          The icon, masked as `resource_mask_table.json` says. An empty image if the
     *                                  icon is a missing PCX image.
     */
    RgbaImage icon(std::string_view filename);

    /**
     * @param filename                  Name of an image in `bitmaps.lod`, case-insensitive. A missing image is
     *                                  replaced with "pending".
     * @return                          The bitmap, with the palette entries `resource_mask_table.json` masks made
     *                                  transparent.
     */
    LodImage bitmap(std::string_view filename);

 private:
    LodReader _eventsLodReader;
    LodReader _iconsLodReader;
    LodReader _bitmapsLodReader;
    ResourceMaskTable _masks;
};
