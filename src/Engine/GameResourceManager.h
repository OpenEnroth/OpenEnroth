#pragma once

#include <string>
#include <memory>

#include "Utility/Memory/Blob.h"

#include "Library/Lod/LodReader.h"

class GameResourceManager {
 public:
    GameResourceManager();
    ~GameResourceManager();

    void openGameResources();

    Blob getEventsFile(std::string_view filename);

 private:
    LodReader _eventsLodReader;
};
