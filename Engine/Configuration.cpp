#include "Engine/Configuration.h"

#include <algorithm>
#include <cctype>

Engine_::Configuration *engine_config = nullptr;

using Engine_::Configuration;

static bool FindCaseInsensitive(const std::string &haystack, const std::string &needle)
{
    auto i = std::search(
        haystack.begin(), haystack.end(),
        needle.begin(), needle.end(),
        [](char c1, char c2) {return std::toupper(c1) == std::toupper(c2); }
    );

    return i != haystack.end();
}

void Configuration::MergeCommandLine(const std::string &cmd)
{
    if (FindCaseInsensitive(cmd, "-window"))
    {
        dword_6BE368_debug_settings_2 |= DEBUG_SETTINGS_RUN_IN_WIDOW;
    }
    if (FindCaseInsensitive(cmd, "-nointro"))
    {
        no_intro = true;
    }
    if (FindCaseInsensitive(cmd, "-nologo"))
    {
        no_logo = true;
    }
    if (FindCaseInsensitive(cmd, "-nosound"))
    {
        no_sound = true;
    }
    if (FindCaseInsensitive(cmd, "-novideo"))
    {
        no_video = true;
    }
    if (FindCaseInsensitive(cmd, "-nowalksound"))
    {
        no_walk_sound = true;
    }
    if (FindCaseInsensitive(cmd, "-nomarg"))
    {
        no_margareth = true;
    }
    if (FindCaseInsensitive(cmd, "-render="))
    {
        if (FindCaseInsensitive(cmd, "-render=DirectDraw"))
            renderer_name = "DirectDraw";
        if (FindCaseInsensitive(cmd, "-render=OpenGL"))
            renderer_name = "OpenGL";
    }
}