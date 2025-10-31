#pragma once

#include <string>

#include "Utility/Memory/Blob.h"

#include "Library/Lod/LodReader.h"

class ResourceManager {
 public:
    ResourceManager();
    ~ResourceManager();

    void open();

    Blob eventsData(std::string_view filename);

 private:
    LodReader _eventsLodReader;
};
