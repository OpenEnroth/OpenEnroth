#pragma once

#include <string>
#include <string_view>

#include "Utility/Memory/Blob.h"

#include "Library/Lod/LodReader.h"

#include "Utility/String/Encoding.h"

/**
 * This class provides access to everything in `/data` folder.
 */
class ResourceManager {
 public:
    ResourceManager();
    ~ResourceManager();

    void open();

    Blob eventsData(std::string_view filename);
    std::string eventsText(std::string_view filename);

    TextEncoding encoding() {
        return _gameDataEncoding;
    }

 private:
    LodReader _eventsLodReader;
    TextEncoding _gameDataEncoding = ENCODING_ASCII;
};
