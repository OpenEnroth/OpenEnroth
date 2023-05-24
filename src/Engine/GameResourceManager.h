#pragma once

#include <string>
#include <memory>

#include "Utility/Memory/Blob.h"
#include "Library/Lod/LodReader.h"

class GameResourceManager {
 public:
    void openGameResources();

    Blob getEventsFile(const std::string &filename);
 private:
    Blob uncompressPseudoTexture(const Blob &input);

    std::unique_ptr<LodReader> _eventsLodReader = nullptr;
};
