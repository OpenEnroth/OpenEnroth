#include "ResourceManager.h"

#include <string>

#include "Library/Json/Json.h"
#include "Library/LodFormats/LodFormats.h"
#include "Library/FileSystem/Interface/FileSystem.h"

#include "Utility/String/Ascii.h"
#include "Utility/MapAccess.h"

#include "EngineFileSystem.h"

ResourceManager::ResourceManager() = default;
ResourceManager::~ResourceManager() = default;

void ResourceManager::open() {
    _eventsLodReader.open(dfs->read("data/events.lod"));
    from_json(Json::parse(dfs->read("data/resource_mask_table.json").str()), _masks);
    // TODO(captainurist):
    //  on exception:
    //      Error(localization->str(LSTR_MIGHT_AND_MAGIC_VII_IS_HAVING_TROUBLE), localization->str(LSTR_REINSTALL_NECESSARY));
    // but we can't use localization object here cause it's not yet initialized.
}

Blob ResourceManager::eventsData(std::string_view filename) {
    return lod::decodeMaybeCompressed(_eventsLodReader.read(filename));
}

ResourceMask ResourceManager::iconMask(std::string_view filename) const {
    return valueOr(_masks.icons, ascii::toLower(filename));
}

ResourceMask ResourceManager::bitmapMask(std::string_view filename) const {
    return valueOr(_masks.bitmaps, ascii::toLower(filename));
}
