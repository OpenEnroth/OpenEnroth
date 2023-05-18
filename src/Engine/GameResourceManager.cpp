#include "Engine/GameResourceManager.h"
#include "Engine/ErrorHandling.h"
#include "Engine/Localization.h"
#include "Engine/Graphics/Image.h"

#include "Utility/DataPath.h"

#include "Library/Compression/Compression.h"

void GameResourceManager::openGameResources() {
    _eventsLodReader = LodReader::open(MakeDataPath("data", "events.lod"), "icons");
    if (!_eventsLodReader) {
        Error(localization->GetString(LSTR_PLEASE_REINSTALL), localization->GetString(LSTR_REINSTALL_NECESSARY));
    }
}

Blob GameResourceManager::getEventsFile(const std::string &filename) {
    assert(_eventsLodReader != nullptr);
    Blob file = _eventsLodReader->read(filename);
    return uncompressPseudoTexture(file);
}

Blob GameResourceManager::uncompressPseudoTexture(Blob &input) {
    const TextureHeader *header = (const TextureHeader*)input.data();
    int compressedSize = header->uTextureSize;
    int uncompressedSize = header->uDecompressedSize;
    if ((input.size() - sizeof(TextureHeader)) < header->uTextureSize) {
        return Blob();
    }
    if (uncompressedSize) {
        return zlib::Uncompress(input.subBlob(sizeof(TextureHeader), header->uTextureSize), header->uDecompressedSize);
    } else {
        return input.subBlob(sizeof(TextureHeader), header->uTextureSize);
    }
}
