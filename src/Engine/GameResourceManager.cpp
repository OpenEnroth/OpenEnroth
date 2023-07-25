#include "Engine/GameResourceManager.h"

#include "Engine/Graphics/Texture_MM7.h"

#include "Library/Compression/Compression.h"

#include "Utility/DataPath.h"
#include "Utility/Streams/BlobInputStream.h"

GameResourceManager::GameResourceManager() = default;
GameResourceManager::~GameResourceManager() = default;

void GameResourceManager::openGameResources() {
    _eventsLodReader.open(makeDataPath("data", "events.lod"));
    // TODO(captainurist):
    //  on exception:
    //      Error(localization->GetString(LSTR_PLEASE_REINSTALL), localization->GetString(LSTR_REINSTALL_NECESSARY));
}

Blob GameResourceManager::getEventsFile(const std::string &filename) {
    Blob file = _eventsLodReader.readRaw(filename);
    return uncompressPseudoTexture(file);
}

Blob GameResourceManager::uncompressPseudoTexture(const Blob &input) {
    BlobInputStream stream(input);
    TextureHeader header;
    deserialize(stream, &header);

    Blob result = stream.readBlobOrFail(header.dataSize);
    if (header.decompressedSize)
        result = zlib::Uncompress(result, header.decompressedSize);
    return result;
}
