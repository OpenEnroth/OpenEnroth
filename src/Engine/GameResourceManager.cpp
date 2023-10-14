#include "Engine/GameResourceManager.h"

#include "Library/LodFormats/LodFormats.h"

#include "Utility/DataPath.h"

GameResourceManager::GameResourceManager() = default;
GameResourceManager::~GameResourceManager() = default;

void GameResourceManager::openGameResources() {
    _eventsLodReader.open(makeDataPath("data", "events.lod"));
    // TODO(captainurist):
    //  on exception:
    //      Error(localization->GetString(LSTR_PLEASE_REINSTALL), localization->GetString(LSTR_REINSTALL_NECESSARY));
}

Blob GameResourceManager::getEventsFile(const std::string &filename) {
    return lod::decodeCompressed(_eventsLodReader.read(filename));
}
