#include "GameOptions.h"

#include <memory>
#include <string>

#include <CLI/CLI.hpp>

#include "Application/GameConfig.h"

using Application::GameConfig;

// TODO(captainurist): this begs a PR to CLI11
template<class T>
bool lexical_cast(const std::string &input, ConfigValue<T> &configValue) {
    T value;
    if (!CLI::detail::lexical_cast(input, value))
        return false;

    configValue.Set(value);
    return true;
}

template<class A, class B, class T>
bool lexical_assign(const std::string &input, ConfigValue<T> &configValue) {
    return lexical_cast(input, configValue);
}

#define MM_DEFINE_CLI_LEXICAL_CAST_FOR_CONFIG_TYPE(TYPE)                                                                \
namespace CLI::detail {                                                                                                 \
    template<>                                                                                                          \
    bool lexical_cast<ConfigValue<TYPE>>(const std::string &input, ConfigValue<TYPE> &configValue) {                    \
        return lexical_cast(input, configValue);                                                                        \
    }                                                                                                                   \
} // namespace CLI::detail

MM_DEFINE_CLI_LEXICAL_CAST_FOR_CONFIG_TYPE(bool)
MM_DEFINE_CLI_LEXICAL_CAST_FOR_CONFIG_TYPE(int)
MM_DEFINE_CLI_LEXICAL_CAST_FOR_CONFIG_TYPE(float)
MM_DEFINE_CLI_LEXICAL_CAST_FOR_CONFIG_TYPE(std::string)

bool Application::ParseGameOptions(int argc, char **argv, GameConfig *config) {
    std::unique_ptr<CLI::App> app = std::make_unique<CLI::App>();

    auto enableDebug = [&] {
        config->debug.ShowFPS.Set(true);
        config->debug.ShowPickedFace.Set(true);
        config->debug.TownPortal.Set(true);
        config->debug.InfiniteFood.Set(true);
        config->debug.InfiniteGold.Set(true);
    };

    auto unset = [] (ConfigValue<bool>& value) {
        return [&] {
            value.Set(false);
        };
    };

    std::string generalOptions = "General Options";
    std::string gameOptions = "Game Options";
    std::string windowOptions = "Window Options";

    app->add_flag("-v,--verbose", config->debug.VerboseLogging, "Enable verbose logging")->group(generalOptions);
    app->set_help_flag("-h,--help", "Print help and exit")->group(generalOptions);

    app->add_flag("--nointro", config->debug.NoIntro, "Skip intro movie")->group(gameOptions);
    app->add_flag("--nologo", config->debug.NoLogo, "Skip 3DO & NWC logos")->group(gameOptions);
    app->add_flag("--nosound", config->debug.NoSound, "Disable sound")->group(gameOptions);
    app->add_flag("--novideo", config->debug.NoVideo, "Disable all videos, including house backgrounds")->group(gameOptions);
    app->add_flag("--nomarg", config->debug.NoMargareth, "Disable Margaret the guide")->group(gameOptions);
    app->add_flag_callback("--nowalksound", unset(config->settings.WalkSound), "Disable walking sound")->group(gameOptions);
    app->add_flag_callback("--nograb", unset(config->window.MouseGrab), "Don't restrict mouse movement to the game window")->group(gameOptions);
    app->add_flag_callback("--debug", enableDebug, "Minimal debug mode")->group(gameOptions);

    app->add_option("--display", config->window.Display, "Display number to place the game window at (0 is your main display)")->type_name("NUMBER")->group(windowOptions);
    app->add_option("--window-width", config->window.Width, "Game window width")->type_name("WIDTH")->group(windowOptions);
    app->add_option("--window-height", config->window.Height, "Game window height")->type_name("HEIGHT")->group(windowOptions);
    app->add_option("--window-x", config->window.PositionX, "Game window x position in display coordinates")->type_name("X")->group(windowOptions);
    app->add_option("--window-y", config->window.PositionY, "Game window y position in display coordinates")->type_name("Y")->group(windowOptions);
    app->add_option("--window-mode", config->window.Mode,
                    "Game window mode, use 0 for windowed, 1 for borderless windowed, 2 for fullscreen and 3 for borderless "
                    "(fake) fullscreen")->check(CLI::Range(0, 3).description(""))->type_name("MODE")->group(windowOptions);

    try {
        app->parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        app->exit(e);
        return false;
    }

    return true;
}
