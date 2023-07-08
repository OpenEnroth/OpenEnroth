#include "GameConfig.h"

#include <filesystem>

#include "Engine/EngineIocContainer.h"

#include "Library/Logger/Logger.h"
#include "Library/Serialization/EnumSerialization.h"

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(RendererType, CASE_INSENSITIVE, {
    {RendererType::OpenGL, "OpenGL"},
    {RendererType::OpenGLES, "OpenGLES"}
})

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(PlatformWindowMode, CASE_INSENSITIVE, {
    {WINDOW_MODE_WINDOWED, "windowed"},
    {WINDOW_MODE_BORDERLESS, "borderless"},
    {WINDOW_MODE_FULLSCREEN, "fullscreen"},
    {WINDOW_MODE_FULLSCREEN_BORDERLESS, "fullscreen_borderless"},
    {WINDOW_MODE_FULLSCREEN_BORDERLESS, "borderless_fullscreen"}, // Alias for convenience

    // Compatibility with old serialization:
    {WINDOW_MODE_WINDOWED, "0"},
    {WINDOW_MODE_BORDERLESS, "1"},
    {WINDOW_MODE_FULLSCREEN, "2"},
    {WINDOW_MODE_FULLSCREEN_BORDERLESS, "3"}
})

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(PlatformLogLevel, CASE_INSENSITIVE, {
    {LOG_VERBOSE, "verbose"},
    {LOG_DEBUG, "debug"},
    {LOG_INFO, "info"},
    {LOG_WARNING, "warning"},
    {LOG_ERROR, "error"},
    {LOG_CRITICAL, "critical"},
})

GameConfig::GameConfig(const std::string &path) : _path(path) {
    _logger = EngineIocContainer::ResolveLogger();
}

GameConfig::~GameConfig() {}

void GameConfig::LoadConfiguration() {
    if (std::filesystem::exists(_path)) {
        Config::load(_path);
        _logger->info("Configuration file '{}' loaded!", _path);
    } else {
        Config::reset();
        _logger->warning("Could not read configuration file '{}'! Loaded default configuration instead!", _path);
    }
}

void GameConfig::SaveConfiguration() {
    Config::save(_path);
    _logger->info("Configuration file '{}' saved!", _path);
}

void GameConfig::resetForTest() {
    reset();

    settings.MusicLevel.setValue(1);
    settings.VoiceLevel.setValue(1);
    settings.SoundLevel.setValue(1);
    debug.NoVideo.setValue(true);
    window.MouseGrab.setValue(false);
    graphics.FPSLimit.setValue(0); // Unlimited
}
