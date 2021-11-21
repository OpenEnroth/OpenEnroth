#include "Engine/AssetLocator.h"

#include "Platform/Api.h"


void AssetLocator::SetBaseGameRoot(AssetClass game_type, const std::string& path) {
    _game_type = game_type;
    _game_path = path;
}

std::string AssetLocator::ResolvePath(const std::string& filename) {
    return _game_path + OS_GetDirSeparator() + filename;
}