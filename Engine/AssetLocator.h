#pragma once

#include <string>


enum AssetClass {
    Mm6Assets,
    Mm7Assets,
    Mm8Assets,
};


class AssetLocator {
public:
    void SetBaseGameRoot(AssetClass game_type, const std::string& path);
    std::string ResolvePath(const std::string& filename);

private:
    AssetClass  _game_type;
    std::string _game_path;
};
