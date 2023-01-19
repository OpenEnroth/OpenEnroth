#include "GameConfig.h"

#include <filesystem>

#include "Engine/IocContainer.h"

#include "Library/Logger/Logger.h"
#include "Library/Serialization/EnumSerialization.h"

#include "Utility/DataPath.h"

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(RendererType, CASE_INSENSITIVE, {
    {RendererType::OpenGL, "OpenGL"},
    {RendererType::OpenGLES, "OpenGLES"},
    {RendererType::DirectDraw, "DirectDraw"}
})

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(PlatformWindowMode, CASE_INSENSITIVE, {
    {WINDOW_MODE_WINDOWED, "windowed"},
    {WINDOW_MODE_BORDERLESS, "borderless"},
    {WINDOW_MODE_FULLSCREEN, "fullscreen"},
    {WINDOW_MODE_FAKE_FULLSCREEN, "fake_fullscreen"},

    // Compatibility with old serialization:
    {WINDOW_MODE_WINDOWED, "0"},
    {WINDOW_MODE_BORDERLESS, "1"},
    {WINDOW_MODE_FULLSCREEN, "2"},
    {WINDOW_MODE_FAKE_FULLSCREEN, "3"}
})

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
