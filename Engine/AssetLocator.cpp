#include <filesystem>

#include "Engine/AssetLocator.h"

#include "Platform/Api.h"


extern std::shared_ptr<AssetLocator> asset_locator = std::make_shared<AssetLocator>();


void AssetLocator::SetBaseGameRoot(AssetClass game_type, const std::string& path) {
    _game_type = game_type;
    _game_path = path;
}

std::string AssetLocator::ResolvePath(const std::string& subpath) {
    return _game_path + OS_GetDirSeparator() + subpath;
}

std::string AssetLocator::LocateDataFile(const std::string& filename) {
    return _game_path + OS_GetDirSeparator() + "data" + OS_GetDirSeparator() + filename;
}

std::string AssetLocator::LocateSaveFileDirectory() {
    std::string saves_dir = _game_path + OS_GetDirSeparator() + "saves";

    // ensure directory structure exists
    std::filesystem::create_directories(saves_dir);
    return saves_dir;
}

std::string AssetLocator::LocateSaveFile(const std::string& filename) {
    return LocateSaveFileDirectory() + OS_GetDirSeparator() + filename;
}

std::string AssetLocator::LocateMusicFile(const std::string& filename) {
    return _game_path + OS_GetDirSeparator() + "music" + OS_GetDirSeparator() + filename;
}

std::string AssetLocator::LocateSoundFile(const std::string& filename) {
    return _game_path + OS_GetDirSeparator() + "sounds" + OS_GetDirSeparator() + filename;
}

std::string AssetLocator::LocateCutsceneFile(const std::string& filename) {
    return _game_path + OS_GetDirSeparator() + "anims" + OS_GetDirSeparator() + filename;
}
