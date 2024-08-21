#include "GameResourceManager.h"

#include "Engine.h"
#include "EngineFileSystem.h"

#include "Library/LodFormats/LodFormats.h"

GameResourceManager::GameResourceManager() = default;
GameResourceManager::~GameResourceManager() = default;

void GameResourceManager::openGameResources() {
    _eventsLodReader.open(dfs->read("data/events.lod"));
    // TODO(captainurist):
    //  on exception:
    //      Error(localization->GetString(LSTR_PLEASE_REINSTALL), localization->GetString(LSTR_REINSTALL_NECESSARY));
    // but we can't use localization object here cause it's not yet initialized.
}

Blob GameResourceManager::getEventsFile(std::string_view filename) {
    return lod::decodeCompressed(_eventsLodReader.read(filename));
}
