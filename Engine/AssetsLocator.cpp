#include <filesystem>

#include "Engine/AssetsLocator.h"

#include "Platform/Api.h"


extern std::shared_ptr<AssetsLocator> assets_locator = std::make_shared<AssetsLocator>();


void AssetsLocator::SetBaseGameRoot(AssetsClass game_type, const std::string& path) {
    _game_type = game_type;
    _game_path = path;
}

std::string AssetsLocator::ResolvePath(const std::string& subpath) {
    return _game_path + OS_GetDirSeparator() + subpath;
}

std::string AssetsLocator::LocateDataFile(const std::string& filename) {
    return _game_path + OS_GetDirSeparator() + "data" + OS_GetDirSeparator() + filename;
}

std::string AssetsLocator::LocateSaveFileDirectory() {
    std::string saves_dir = _game_path + OS_GetDirSeparator() + "saves";

    // ensure directory structure exists
    std::filesystem::create_directories(saves_dir);
    return saves_dir;
}

std::string AssetsLocator::LocateSaveFile(const std::string& filename) {
    return LocateSaveFileDirectory() + OS_GetDirSeparator() + filename;
}

std::string AssetsLocator::LocateMusicFile(const std::string& filename) {
    return _game_path + OS_GetDirSeparator() + "music" + OS_GetDirSeparator() + filename;
}

std::string AssetsLocator::LocateSoundFile(const std::string& filename) {
    return _game_path + OS_GetDirSeparator() + "sounds" + OS_GetDirSeparator() + filename;
}

std::string AssetsLocator::LocateCutsceneFile(const std::string& filename) {
    return _game_path + OS_GetDirSeparator() + "anims" + OS_GetDirSeparator() + filename;
}
