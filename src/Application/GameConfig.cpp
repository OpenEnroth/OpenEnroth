#include "GameConfig.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

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
});

GameConfig::GameConfig() = default;
GameConfig::~GameConfig() = default;

static constexpr std::initializer_list<const char *> defaultCommands = {
    "config toggle town_portal | Town Portal",
    "config toggle wizard_eye | Wizard Eye",
    "config toggle all_magic | All Magic",
    "config toggle terrain | Terrain",
    "config toggle lightmap_decals | Lightmap Decals",
    "config toggle turbo_speed | Turbo Speed",
    "config toggle no_actors | No Actors",
    "config toggle no_damage | No Damage",
    "config toggle fog | Toggle Fog",
    "config toggle snow | Toggle Snow",
    "config toggle seasons_change | Toggle Seasons",
    "config toggle show_fps | Show FPS",
    "config toggle show_picked_face | Picked Face",
    "config toggle portal_outlines | Portal Outlines",
    "gold add 10000 | Add 10k Gold",
    "gold set 0 | Remove all Gold",
    "food add 20 | Add 20 Food",
    "food set 0 | Remove all Food",
    "xp party 20000 | Give 20k xp",
    "sp add 20 | Add 20 Skill points",
    "skills learn_all | Learn all Skills",
    "alignment cycle | Cycle Alignment",
    "class lich | Become Lich",
    "condition set Dead | Become Dead",
    "condition set Eradicated | Become Eradicated",
    "inventory addrandom | Add Random Item",
    "inventory addrandomspecial | Add Special Item",
    "reload_shaders | Reload Shaders",
    "hp full_heal | Full Heal"
};

GameConfig::CheatCommands::CheatCommands(GameConfig *config) : ConfigSection(config, "cheat_commands") {
    // I'm adding 40 in case we want to add more commands even if we provide 29 by default
    static const int maxNumberOfCommands = 40;
    int i = 0;
    for (const char* defaultCommand : defaultCommands) {
        _addCommand(i, defaultCommand);
        ++i;
    }
    for (; i < maxNumberOfCommands; ++i) {
        _addCommand(i, "");
    }
}

void GameConfig::CheatCommands::_addCommand(int commandIndex, const std::string& defaultValue) {
    std::string name = fmt::format("command{:02}", commandIndex + 1);
    auto item = std::make_unique<String>(this, name, defaultValue,
        "Cheat Command. Example: 'xp add 1000|Give 1000 xp to current Character'");
    CommandList.push_back(std::move(item));
}
