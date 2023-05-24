#include "Engine/GameResourceManager.h"
#include "Engine/ErrorHandling.h"
#include "Engine/Localization.h"
#include "Engine/Graphics/Image.h"

#include "Utility/DataPath.h"
#include "Utility/Streams/BlobInputStream.h"

#include "Library/Compression/Compression.h"

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

    int uncompressedSize = header.uDecompressedSize;
    assert((input.size() - sizeof(TextureHeader)) == header.uTextureSize); // TODO(captainurist): throw?
    if (uncompressedSize) {
        return zlib::Uncompress(stream.tail(), header.uDecompressedSize);
    } else {
        return stream.tail();
    }
}
