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
        log->Warning("x16 Turn Speed");  // really shouldn't use this mode
        cfg->turn_speed = 128;
        break;

    case 2:             // 32x
        log->Warning("x32 Turn Speed");  // really shouldn't use this mode
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

std::shared_ptr<EngineConfig> EngineConfigFactory::Create(std::shared_ptr<CommandLine> command_line) {
    auto config = CreateDefaultConfiguration();

    std::shared_ptr<std::string> value;
    if (command_line->TryFindKey("-window")) {
        config->run_in_window |= DEBUG_SETTINGS_RUN_IN_WIDOW;
    }
    if (command_line->TryFindKey("-nointro")) {
        config->no_intro = true;
    }
    if (command_line->TryFindKey("-nologo")) {
        config->no_logo = true;
    }
    if (command_line->TryFindKey("-nosound")) {
        config->no_sound = true;
    }
    if (command_line->TryFindKey("-novideo")) {
        config->no_video = true;
    }
    if (command_line->TryFindKey("-nowalksound")) {
        config->no_walk_sound = true;
    }
    if (command_line->TryFindKey("-nomarg")) {
        config->no_margareth = true;
    }
    if (command_line->TryFindKey("-nograb")) {
        config->no_grab = true;
    }
    if (command_line->TryGetValue("render", &value)) {
        config->renderer_name = *value;
    }
    if (command_line->TryGetValue("display", &value)) {
        config->display = std::stoi(*value);
    }
    if (command_line->TryGetValue("window_width", &value)) {
        config->window_width = std::stoi(*value);
    }
    if (command_line->TryGetValue("window_height", &value)) {
        config->window_height = std::stoi(*value);
    }
    if (command_line->TryGetValue("window_x", &value)) {
        config->window_x = std::stoi(*value);
    }
    if (command_line->TryGetValue("window_y", &value)) {
        config->window_y = std::stoi(*value);
    }
    if (command_line->TryGetValue("fullscreen", &value)) {
        config->fullscreen = std::stoi(*value);
    }
    if (command_line->TryGetValue("borderless", &value)) {
        config->borderless = std::stoi(*value);
    }

    return config;
}
