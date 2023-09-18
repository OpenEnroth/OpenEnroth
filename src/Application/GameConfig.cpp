#include "GameConfig.h"

#include <filesystem>

#include "Engine/EngineIocContainer.h"

#include "Library/Logger/Logger.h"
#include "Library/Serialization/EnumSerialization.h"

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

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(RandomEngineType, CASE_INSENSITIVE, {
    {RANDOM_ENGINE_MERSENNE_TWISTER, "mersenne_twister"},
    {RANDOM_ENGINE_SEQUENTIAL, "sequential"}
})

GameConfig::GameConfig() = default;
GameConfig::~GameConfig() = default;

void GameConfig::resetForTest() {
    RendererType rendererType = graphics.Renderer.value();
    reset();
    graphics.Renderer.setValue(rendererType); // TODO(captainurist): this is so questionable... Why not keep all other graphics settings then?

    settings.MusicLevel.setValue(1);
    settings.VoiceLevel.setValue(1);
    settings.SoundLevel.setValue(1);
    debug.NoVideo.setValue(true);
    window.MouseGrab.setValue(false);
    graphics.FPSLimit.setValue(0); // Unlimited
}
