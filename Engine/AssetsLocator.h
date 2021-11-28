#pragma once

#include <string>
#include <memory>

#include "Engine/AssetsClass.h"


class AssetsLocator {
public:
    void SetBaseGameRoot(AssetsClass game_type, const std::string& path);
    std::string ResolvePath(const std::string& subpath);
    std::string LocateDataFile(const std::string& filename);
    std::string LocateSaveFileDirectory();
    std::string LocateSaveFile(const std::string& filename);
    std::string LocateSoundFile(const std::string& filename);
    std::string LocateMusicFile(const std::string& filename);
    std::string LocateCutsceneFile(const std::string& filename);

private:
    AssetsClass _game_type;
    std::string _game_path;
};


extern std::shared_ptr<AssetsLocator> assets_locator;
