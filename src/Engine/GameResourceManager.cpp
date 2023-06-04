#include "Engine/GameResourceManager.h"

#include "Engine/ErrorHandling.h"
#include "Engine/Localization.h"
#include "Engine/Graphics/Texture_MM7.h"

#include "Library/Compression/Compression.h"

#include "Utility/DataPath.h"
#include "Utility/Streams/BlobInputStream.h"

void GameResourceManager::openGameResources() {
    _eventsLodReader = LodReader::open(makeDataPath("data", "events.lod"));
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
    BlobInputStream stream(input);
    TextureHeader header;
    deserialize(stream, &header);

    Blob result = stream.readBlobOrFail(header.uTextureSize);
    if (header.uDecompressedSize)
        result = zlib::Uncompress(result, header.uDecompressedSize);
    return result;
}
