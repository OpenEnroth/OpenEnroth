#include "GameConfig.h"

#include <filesystem>

#include "Engine/IocContainer.h"

#include "Library/Logger/Logger.h"

#include "Utility/DataPath.h"

using EngineIoc = Engine_::IocContainer;
using Application::GameConfig;

void GameConfig::LoadConfiguration() {
    std::string path = MakeDataPath(config_file);

    if (std::filesystem::exists(path)) {
        Config::Load(path);
        logger->Info("Configuration file '%s' loaded!", path.c_str());
    } else {
        Config::Reset();
        logger->Warning("Cound not read configuration file '%s'! Loaded default configuration instead!", path.c_str());
    }
}

void GameConfig::SaveConfiguration() {
    std::string path = MakeDataPath(config_file);

    Config::Save(path);
    logger->Info("Configuration file '%s' saved!", path.c_str());
}

GameConfig::GameConfig() {
    this->logger = EngineIoc::ResolveLogger();
}

GameConfig::~GameConfig() {}
