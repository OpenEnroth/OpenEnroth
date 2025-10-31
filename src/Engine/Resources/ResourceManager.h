#pragma once

#include <string_view>

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

 private:
    LodReader _eventsLodReader;
};
