#include "Engine/EngineConfigFactory.h"

#include <algorithm>
#include <cctype>

#include "Platform/Api.h"

using Engine_::EngineConfigFactory;
using Engine_::EngineConfig;


std::shared_ptr<EngineConfig> EngineConfigFactory::CreateDefaultConfiguration() {
    auto cfg = std::make_shared<EngineConfig>();

    if (!cfg->no_walk_sound) {
        cfg->no_walk_sound = OS_GetAppInt("WalkSound", 1) == 0;
    }
    if (!cfg->always_run) {
        cfg->always_run = OS_GetAppInt("valAlwaysRun", 0) != 0;
    }
    cfg->flip_on_exit = OS_GetAppInt("FlipOnExit", 0) != 0;

    cfg->show_damage = OS_GetAppInt("ShowDamage", 1) != 0;
    int turn_type = OS_GetAppInt("TurnDelta", 3);

    switch (turn_type) {
    case 1:             // 16x
        log->Warning(L"x16 Turn Speed");  // really shouldn't use this mode
        cfg->turn_speed = 128;
        break;

    case 2:             // 32x
        log->Warning(L"x32 Turn Speed");  // really shouldn't use this mode
        cfg->turn_speed = 64;
        break;

    case 3:             // smooth
    default:
        cfg->turn_speed = 0;
        break;
    }

    cfg->sound_level = std::min(4, OS_GetAppInt("soundflag", 9));
    cfg->music_level = std::min(3, OS_GetAppInt("musicflag", 9));
    cfg->voice_level = std::min(4, OS_GetAppInt("CharVoices", 9));

    cfg->gamma = std::min(4, OS_GetAppInt("GammaPos", 4));

    if (OS_GetAppInt("Bloodsplats", 1))
        cfg->flags2 |= GAME_FLAGS_2_DRAW_BLOODSPLATS;
    else
        cfg->flags2 &= ~GAME_FLAGS_2_DRAW_BLOODSPLATS;
    cfg->no_bloodsplats = !(cfg->flags2 & GAME_FLAGS_2_DRAW_BLOODSPLATS);

    return cfg;
}


static bool FindCaseInsensitive(const std::string &haystack, const std::string &needle) {
    auto i = std::search(
        haystack.begin(), haystack.end(),
        needle.begin(), needle.end(),
        [](char c1, char c2) {return std::toupper(c1) == std::toupper(c2); });

    return i != haystack.end();
}


std::shared_ptr<EngineConfig> EngineConfigFactory::Clone(std::shared_ptr<const EngineConfig> other) {
    return std::make_shared<EngineConfig>(*other.get());
}

std::shared_ptr<EngineConfig> EngineConfigFactory::Mutate(
    std::shared_ptr<const EngineConfig> config,
    std::function<void(std::shared_ptr<EngineConfig> &)> mutator
) {
    auto new_config = std::make_shared<EngineConfig>(*config.get());
    mutator(new_config);

    return new_config;
}

std::shared_ptr<EngineConfig> EngineConfigFactory::Create() {
    return CreateDefaultConfiguration();
}

std::shared_ptr<EngineConfig> EngineConfigFactory::CreateFromCommandLine(const std::string &cmd) {
    auto config = CreateDefaultConfiguration();

    // config->renderer_name = "OpenGL";

    if (FindCaseInsensitive(cmd, "-window")) {
        config->dword_6BE368_debug_settings_2 |= DEBUG_SETTINGS_RUN_IN_WIDOW;
    }
    if (FindCaseInsensitive(cmd, "-nointro")) {
        config->no_intro = true;
    }
    if (FindCaseInsensitive(cmd, "-nologo")) {
        config->no_logo = true;
    }
    if (FindCaseInsensitive(cmd, "-nosound")) {
        config->no_sound = true;
    }
    if (FindCaseInsensitive(cmd, "-novideo")) {
        config->no_video = true;
    }
    if (FindCaseInsensitive(cmd, "-nowalksound")) {
        config->no_walk_sound = true;
    }
    if (FindCaseInsensitive(cmd, "-nomarg")) {
        config->no_margareth = true;
    }
    if (FindCaseInsensitive(cmd, "-render=")) {
        if (FindCaseInsensitive(cmd, "-render=DirectDraw")) {
            config->renderer_name = "DirectDraw";
        }
        if (FindCaseInsensitive(cmd, "-render=OpenGL")) {
            config->renderer_name = "OpenGL";
        }
    }

    return config;
}
