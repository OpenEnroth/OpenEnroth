#include "Engine/EngineFactory.h"

#include <algorithm>
#include <cctype>

#include "Platform/Api.h"

using Engine_::EngineFactory;
using Engine_::Configuration;


std::shared_ptr<Engine> EngineFactory::CreateEngine(const std::string &command_line) {
    auto config = CreateDefaultConfiguration();
    config = MergeCommandLine(config, command_line);

    auto engine = std::make_shared<Engine>();
    if (engine) {
        engine->Configure(config);
    }
    return engine;
}

std::shared_ptr<Configuration> EngineFactory::CreateDefaultConfiguration() {
    auto cfg = std::make_shared<Configuration>();

    if (!cfg->no_walk_sound) {
        cfg->no_walk_sound = OS_GetAppInt("WalkSound", 1) == 0;
    }
    cfg->always_run = OS_GetAppInt("valAlwaysRun", 0) != 0;
    cfg->flip_on_exit = OS_GetAppInt("FlipOnExit", 0) != 0;

    cfg->show_damage = OS_GetAppInt("ShowDamage", 1) != 0;
    int turn_type = OS_GetAppInt("TurnDelta", 3);

    switch (turn_type) {
    case 1:             // 16x
        logger->Warning(L"x16 Turn Speed");  // really shouldn't use this mode
        cfg->turn_speed = 128;
        break;

    case 2:             // 32x
        logger->Warning(L"x32 Turn Speed");  // really shouldn't use this mode
        cfg->turn_speed = 64;
        break;

    case 3:             // smooth
    default:
        cfg->turn_speed = 0;
        break;
    }

    cfg->sound_level = min(9, OS_GetAppInt("soundflag", 9));
    cfg->music_level = min(9, OS_GetAppInt("musicflag", 9));
    cfg->voice_level = min(9, OS_GetAppInt("CharVoices", 9));

    cfg->gamma = min(4, OS_GetAppInt("GammaPos", 4));

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


std::shared_ptr<Configuration> EngineFactory::MergeCommandLine(
    std::shared_ptr<Configuration> config,
    const std::string &cmd
) {
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
}