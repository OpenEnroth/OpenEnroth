#include "Engine/GameResourceManager.h"
#include "Engine/ErrorHandling.h"
#include "Engine/Localization.h"
#include "Engine/Graphics/Image.h"

#include "Utility/DataPath.h"
#include "Utility/Streams/MemoryInputStream.h"

#include "Library/Compression/Compression.h"

void GameResourceManager::openGameResources() {
    _eventsLodReader = LodReader::open(MakeDataPath("data", "events.lod"));
    if (!_eventsLodReader) {
        Error(localization->GetString(LSTR_PLEASE_REINSTALL), localization->GetString(LSTR_REINSTALL_NECESSARY));
    }
}

Blob GameResourceManager::getEventsFile(const std::string &filename) {
    assert(_eventsLodReader != nullptr);
    Blob file = _eventsLodReader->read(filename);
    return uncompressPseudoTexture(file);
}

Blob GameResourceManager::uncompressPseudoTexture(const Blob &input) {
    MemoryInputStream stream(input.data(), input.size());
    TextureHeader header;
    deserialize(stream, &header);

    int compressedSize = header.uTextureSize;
    int uncompressedSize = header.uDecompressedSize;
    assert((input.size() - sizeof(TextureHeader)) >= header.uTextureSize);
    if (uncompressedSize) {
        return zlib::Uncompress(input.subBlob(sizeof(TextureHeader), header.uTextureSize), header.uDecompressedSize);
    } else {
        return input.subBlob(sizeof(TextureHeader), header.uTextureSize);
    }
}
