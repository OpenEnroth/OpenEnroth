#pragma once

#include <string>
#include <memory>


enum AssetClass {
    Mm6Assets,
    Mm7Assets,
    Mm8Assets,
};


class AssetLocator {
public:
    void SetBaseGameRoot(AssetClass game_type, const std::string& path);
    std::string ResolvePath(const std::string& subpath);
    std::string LocateDataFile(const std::string& filename);
    std::string LocateSaveFileDirectory();
    std::string LocateSaveFile(const std::string& filename);
    std::string LocateSoundFile(const std::string& filename);
    std::string LocateMusicFile(const std::string& filename);
    std::string LocateCutsceneFile(const std::string& filename);

private:
    AssetClass  _game_type;
    std::string _game_path;
};


extern std::shared_ptr<AssetLocator> asset_locator;
