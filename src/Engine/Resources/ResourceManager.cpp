#include "ResourceManager.h"

#include <string>

#include "Library/LodFormats/LodFormats.h"
#include "Library/FileSystem/Interface/FileSystem.h"

#include "EngineFileSystem.h"
#include "Library/EncodingDetector/EncodingDetector.h"

ResourceManager::ResourceManager() = default;
ResourceManager::~ResourceManager() = default;

void ResourceManager::open() {
    _eventsLodReader.open(dfs->read("data/events.lod"));
    // TODO(captainurist):
    //  on exception:
    //      Error(localization->str(LSTR_MIGHT_AND_MAGIC_VII_IS_HAVING_TROUBLE), localization->str(LSTR_REINSTALL_NECESSARY));
    //  but we can't use localization object here cause it's not yet initialized.

    EncodingDetector detector;
    detector.write(eventsData("history.txt").str());
    TextEncoding result = detector.finish();
    _gameDataEncoding = (result != ENCODING_BYTES) ? result : ENCODING_ASCII;
}

Blob ResourceManager::eventsData(std::string_view filename) {
    return lod::decodeMaybeCompressed(_eventsLodReader.read(filename));
}

std::string ResourceManager::eventsText(std::string_view filename) {
    return txt::encodedToUtf8(eventsData(filename).str(), _gameDataEncoding);
}
