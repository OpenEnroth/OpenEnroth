#pragma once

#include <string_view>

#include "Engine/Data/ResourceMask.h"
#include "Engine/Tables/ResourceMaskTable.h"

#include "Utility/Memory/Blob.h"

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
     * @return                          Mask to apply to that image.
     */
    ResourceMask iconMask(std::string_view filename) const;

    /**
     * @param filename                  Name of an image in `bitmaps.lod`, case-insensitive.
     * @return                          Mask to apply to that image.
     */
    ResourceMask bitmapMask(std::string_view filename) const;

 private:
    LodReader _eventsLodReader;
    ResourceMaskTable _masks;
};
