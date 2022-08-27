#include "src/Application/GameConfig.h"

#include <algorithm>
#include <cctype>
#include <string>

#include "src/tools/DataPath.h"

#define MINI_CASE_SENSITIVE
#include "thirdparty/mini/src/mini/ini.h"

using Application::GameConfig;

mINI::INIStructure ini;

int LoadOptionInteger(std::string section, std::string key, int def_value) {
    std::string v = ini[section].get(key);
    if (v.empty())
        return def_value;

    return std::stoi(v);
}

float LoadOptionFloat(std::string section, std::string key, float def_value) {
    std::string v = ini[section].get(key);
    if (v.empty())
        return def_value;

    return std::stof(v);
}

bool LoadOptionBool(std::string section, std::string key, bool def_value) {
    std::string v = ini[section].get(key);
    if (v.empty())
        return def_value;

    if (v == "false" || v == "0")
        return false;

    return true;
}

std::string LoadOptionString(std::string section, std::string key, std::string def_value) {
    std::string v = ini[section].get(key);
    if (v.empty())
        return def_value;

    return v;
}

void SaveOption(std::string section, std::string key, std::string value) {
    ini[section].set(key, value);
}

void SaveOption(std::string section, std::string key, int value) {
    ini[section].set(key, std::to_string(value));
}

void SaveOption(std::string section, std::string key, float value) {
    ini[section].set(key, std::to_string(value));
}

void SaveOption(std::string section, std::string key, bool value) {
    std::string v = "false";
    if (value)
        v = "true";

    ini[section].set(key, v);
}

void GameConfig::DefaultConfiguration() {
    graphics.extended_draw_distance = false;

    debug.Default();
    gameplay.Default();
    graphics.Default();
    keybindings.Default();
    settings.Default();
    window.Default();
}

inline std::string BoolStr(bool value) {
    return value ? "true" : "false";
}

void GameConfig::LoadConfiguration() {
    mINI::INIFile file(MakeDataPath(config_file));

    if (file.read(ini)) {
        debug.Load();
        gameplay.Load();
        graphics.Load();
        keybindings.Load();
        settings.Load();
        window.Load();

        printf("Configuration file '%s' loaded!\n", config_file.c_str());
    } else {
        DefaultConfiguration();
        printf("Cound not read configuration file '%s'! Loaded default configuration instead!\n", config_file.c_str());
    }
}

void GameConfig::SaveConfiguration() {
    mINI::INIFile file(MakeDataPath(config_file));

    debug.Save();
    gameplay.Save();
    graphics.Save();
    keybindings.Save();
    settings.Save();
    window.Save();

    file.write(ini, true);
}

GameConfig::GameConfig(const std::string &command_line_str) {
    command_line = std::make_shared<CommandLine>(command_line_str);

    LoadConfiguration();

    std::shared_ptr<std::string> value;
    if (command_line->TryFindKey("-nointro")) {
        debug.SetNoIntro(true);
    }
    if (command_line->TryFindKey("-nologo")) {
        debug.SetNoLogo(true);
    }
    if (command_line->TryFindKey("-nosound")) {
        debug.SetNoSound(true);
    }
    if (command_line->TryFindKey("-novideo")) {
        debug.SetNoVideo(true);
    }
    if (command_line->TryFindKey("-nomarg")) {
        debug.SetNoMargareth(true);
    }
    if (command_line->TryGetValue("render", &value)) {
        graphics.SetRenderer(*value);
    }
    if (command_line->TryFindKey("-nowalksound")) {
        settings.SetWalkSound(false);
    }
    if (command_line->TryFindKey("-nograb")) {
        window.SetMouseGrab(false);
    }
    if (command_line->TryGetValue("display", &value)) {
        window.SetDisplay(std::stoi(*value));
    }
    if (command_line->TryGetValue("window_width", &value)) {
        window.SetWidth(std::stoi(*value));
    }
    if (command_line->TryGetValue("window_height", &value)) {
        window.SetHeight(std::stoi(*value));
    }
    if (command_line->TryGetValue("window_x", &value)) {
        window.SetPositionX(std::stoi(*value));
    }
    if (command_line->TryGetValue("window_y", &value)) {
        window.SetPositionY(std::stoi(*value));
    }
    if (command_line->TryGetValue("fullscreen", &value)) {
        window.SetFullscreen(std::stoi(*value));
    }
    if (command_line->TryGetValue("borderless", &value)) {
        window.SetBorderless(std::stoi(*value));
    }
    // minimal debug, w/o full magic etc.
    if (command_line->TryFindKey("-debug")) {
        debug.SetShowFPS(true);
        debug.SetShowPickedFace(true);
        debug.SetTownPortal(true);
        debug.SetInfiniteFood(true);
        debug.SetInfiniteGold(true);
    }
}

bool GameConfig::SectionCheck(std::string section) {
    if (section == debug.section_name || section == gameplay.section_name || section == graphics.section_name ||
        section == keybindings.section_name || section == settings.section_name || section == window.section_name)
        return false;

    return true;
}

bool GameConfig::SetCustomString(std::string section, std::string key, std::string value) {
    if (!SectionCheck(section))
        return false;

    SaveOption(section, key, value);

    return true;
}
bool GameConfig::SetCustomBool(std::string section, std::string key, bool value) {
    if (!SectionCheck(section))
        return false;

    SaveOption(section, key, value);

    return true;
}
bool GameConfig::SetCustomInteger(std::string section, std::string key, int value) {
    if (!SectionCheck(section))
        return false;

    SaveOption(section, key, value);

    return true;
}
bool GameConfig::SetCustomFloat(std::string section, std::string key, float value) {
    if (!SectionCheck(section))
        return false;

    SaveOption(section, key, value);

    return true;
}

std::string GameConfig::GetCustomString(std::string section, std::string key, std::string def_value) {
    return LoadOptionString(section, key, def_value);
}

bool GameConfig::GetCustomBool(std::string section, std::string key, bool def_value) {
    return LoadOptionBool(section, key, def_value);
}

int GameConfig::GetCustomInteger(std::string section, std::string key, int def_value) {
    return LoadOptionInteger(section, key, def_value);
}

float GameConfig::GetCustomFloat(std::string section, std::string key, float def_value) {
    return LoadOptionFloat(section, key, def_value);
}

void GameConfig::Debug::Default() {
    SetAllMagic(DefaultAllMagic());
    SetInfiniteFood(DefaultInfiniteFood());
    SetInfiniteGold(DefaultInfiniteGold());
    SetLightmapDecals(DefaultLightmapDecals());
    SetPortalOutlines(DefaultPortalOutlines());
    SetTerrain(DefaultTerrain());
    SetTownPortal(DefaultTownPortal());
    SetTurboSpeed(DefaultTurboSpeed());
    SetWizardEye(DefaultWizardEye());
    SetShowFPS(DefaultShowFPS());
    SetShowPickedFace(DefaultShowPickedFace());
    SetNoActors(DefaultNoActors());
    SetNoDamage(DefaultNoDamage());
    SetNoDecorations(DefaultNoDecorations());
    SetNoIntro(DefaultNoIntro());
    SetNoLogo(DefaultNoLogo());
    SetNoMargareth(DefaultNoMargareth());
    SetNoSound(DefaultNoSound());
    SetNoVideo(DefaultNoVideo());
    SetVerboseLogging(DefaultVerboseLogging());
}

void GameConfig::Debug::Load() {
    SetAllMagic(LoadOptionBool(section_name, "all_magic", DefaultAllMagic()));
    SetInfiniteFood(LoadOptionBool(section_name, "infinite_food", DefaultInfiniteFood()));
    SetInfiniteGold(LoadOptionBool(section_name, "infinite_gold", DefaultInfiniteGold()));
    SetLightmapDecals(LoadOptionBool(section_name, "lightmap_decals", DefaultLightmapDecals()));
    SetTerrain(LoadOptionBool(section_name, "terrain", DefaultTerrain()));
    SetTownPortal(LoadOptionBool(section_name, "town_portal", DefaultTownPortal()));
    SetTurboSpeed(LoadOptionBool(section_name, "turbo_speed", DefaultTurboSpeed()));
    SetWizardEye(LoadOptionBool(section_name, "wizard_eye", DefaultWizardEye()));
    SetShowFPS(LoadOptionBool(section_name, "show_fps", DefaultShowFPS()));
    SetShowPickedFace(LoadOptionBool(section_name, "show_picked_face", DefaultShowPickedFace()));
    SetNoActors(LoadOptionBool(section_name, "no_actors", DefaultNoActors()));
    SetNoDamage(LoadOptionBool(section_name, "no_damage", DefaultNoDamage()));
    SetNoDecorations(LoadOptionBool(section_name, "no_decorations", DefaultNoDecorations()));
    SetNoIntro(LoadOptionBool(section_name, "no_intro", DefaultNoIntro()));
    SetNoLogo(LoadOptionBool(section_name, "no_logo", DefaultNoLogo()));
    SetNoMargareth(LoadOptionBool(section_name, "no_margareth", DefaultNoMargareth()));
    SetNoSound(LoadOptionBool(section_name, "no_sound", DefaultNoSound()));
    SetNoVideo(LoadOptionBool(section_name, "no_video", DefaultNoVideo()));
    SetVerboseLogging(LoadOptionBool(section_name, "verbose_logging", DefaultVerboseLogging()));
}

void GameConfig::Debug::Save() {
    SaveOption(section_name, "all_magic", GetAllMagic());
    SaveOption(section_name, "infinite_food", GetInfiniteFood());
    SaveOption(section_name, "infinite_gold", GetInfiniteGold());
    SaveOption(section_name, "lightmaps_decals", GetLightmapDecals());
    SaveOption(section_name, "town_portal", GetTownPortal());
    SaveOption(section_name, "turbo_speed", GetTurboSpeed());
    SaveOption(section_name, "wizard_eye", GetWizardEye());
    SaveOption(section_name, "show_fps", GetShowFPS());
    SaveOption(section_name, "show_picked_face", GetShowPickedFace());
    SaveOption(section_name, "no_actors", GetNoActors());
    SaveOption(section_name, "no_damage", GetNoDamage());
    SaveOption(section_name, "no_decorations", GetNoDecorations());
    SaveOption(section_name, "no_intro", GetNoIntro());
    SaveOption(section_name, "no_logo", GetNoLogo());
    SaveOption(section_name, "no_margareth", GetNoMargareth());
    SaveOption(section_name, "no_sound", GetNoSound());
    SaveOption(section_name, "no_video", GetNoVideo());
    SaveOption(section_name, "verbose_logging", GetVerboseLogging());
}

void GameConfig::Gameplay::Default() {
    SetMaxFlightHeight(DefaultMaxFlightHeight());
    SetArtifactLimit(DefaultArtifactLimit());
    SetMouseInteractionDepth(DefaultMouseInteractionDepth());
    SetKeyboardInteractionDepth(DefaultKeyboardInteractionDepth());
    SetMouseInfoDepthIndoor(DefaultMouseInfoDepthIndoor());
    SetMouseInfoDepthOutdoor(DefaultMouseInfoDepthOutdoor());
    SetRangedAttackDepth(DefaultRangedAttackDepth());
    SetFloorChecksEps(DefaultFloorChecksEps());
    SetShowUndentifiedItem(DefaultShowUndentifiedItem());
}

void GameConfig::Gameplay::Load() {
    SetMaxFlightHeight(LoadOptionInteger(section_name, "max_flight_height", DefaultMaxFlightHeight()));
    SetArtifactLimit(LoadOptionInteger(section_name, "artifact_limit", DefaultArtifactLimit()));
    SetMouseInteractionDepth(LoadOptionFloat(section_name, "mouse_interaction_depth", DefaultMouseInteractionDepth()));
    SetKeyboardInteractionDepth(LoadOptionFloat(section_name, "keyboard_interaction_depth", DefaultKeyboardInteractionDepth()));
    SetMouseInfoDepthIndoor(LoadOptionFloat(section_name, "mouse_info_depth_indoor", DefaultMouseInfoDepthIndoor()));
    SetMouseInfoDepthOutdoor(LoadOptionFloat(section_name, "mouse_info_depth_outdoor", DefaultMouseInfoDepthOutdoor()));
    SetRangedAttackDepth(LoadOptionFloat(section_name, "ranged_attack_depth", DefaultRangedAttackDepth()));
    SetFloorChecksEps(LoadOptionInteger(section_name, "floor_checks_eps", DefaultFloorChecksEps()));
    SetShowUndentifiedItem(LoadOptionBool(section_name, "show_unidentified_item", DefaultShowUndentifiedItem()));
}

void GameConfig::Gameplay::Save() {
    SaveOption(section_name, "max_flight_height", GetMaxFlightHeight());
    SaveOption(section_name, "artifact_limit", GetArtifactLimit());
    SaveOption(section_name, "mouse_interaction_depth", GetMouseInteractionDepth());
    SaveOption(section_name, "keyboard_interaction_depth", GetKeyboardInteractionDepth());
    SaveOption(section_name, "mouse_info_depth_indoor", GetMouseInfoDepthIndoor());
    SaveOption(section_name, "mouse_info_depth_outdoor", GetMouseInfoDepthOutdoor());
    SaveOption(section_name, "ranged_attack_depth", GetRangedAttackDepth());
    SaveOption(section_name, "floor_checks_eps", GetFloorChecksEps());
    SaveOption(section_name, "show_unidentified_item", GetShowUndentifiedItem());
}

void GameConfig::Graphics::Default() {
    SetRenderer(DefaultRenderer());
    SetBloodSplats(DefaultBloodSplats());
    SetColoredLights(DefaultColoredLight());
    SetTinting(DefaultTinting());
    SetHWLBitmaps(DefaultHWLBitmaps());
    SetHWLSprites(DefaultHWLSprites());
    SetGamma(DefaultGamma());
    SetSnow(DefaultSnow());
    SetSeasonsChange(DefaultSeasonsChange());
    SetSpecular(DefaultFog());
    SetFog(DefaultFog());
}

void GameConfig::Graphics::Load() {
    SetRenderer(LoadOptionString(section_name, "renderer", DefaultRenderer()));
    SetBloodSplats(LoadOptionBool(section_name, "bloodsplats", DefaultBloodSplats()));
    SetColoredLights(LoadOptionBool(section_name, "tinting", DefaultColoredLight()));
    SetTinting(LoadOptionBool(section_name, "tinting", DefaultTinting()));
    SetHWLBitmaps(LoadOptionBool(section_name, "hwl_bitmaps", DefaultHWLBitmaps()));
    SetHWLSprites(LoadOptionBool(section_name, "hwl_sprites", DefaultHWLSprites()));
    SetGamma(LoadOptionInteger(section_name, "gamma", DefaultGamma()));
    SetSnow(LoadOptionBool(section_name, "snow", DefaultSnow()));
    SetSeasonsChange(LoadOptionBool(section_name, "seasons_change", DefaultSeasonsChange()));
    SetSpecular(LoadOptionBool(section_name, "specular", DefaultSpecular()));
    SetFog(LoadOptionBool(section_name, "fog", DefaultFog()));
}

void GameConfig::Graphics::Save() {
    SaveOption(section_name, "renderer", GetRenderer());
    SaveOption(section_name, "bloodsplats", GetBloodSplats());
    SaveOption(section_name, "colored_lights", GetColoredLights());
    SaveOption(section_name, "tinting", GetTinting());
    SaveOption(section_name, "hwl_bitmaps", GetHWLBitmaps());
    SaveOption(section_name, "hwl_sprites", GetHWLSprites());
    SaveOption(section_name, "gamma", GetGamma());
    SaveOption(section_name, "snow", GetSnow());
    SaveOption(section_name, "seasons_change", GetSeasonsChange());
    SaveOption(section_name, "specular", GetSpecular());
    SaveOption(section_name, "fog", GetFog());
}

void GameConfig::Keybindings::Default() {
    SetForward(DefaultForward());
    SetBackward(DefaultBackward());
    SetLeft(DefaultLeft());
    SetRight(DefaultRight());
    SetAttack(DefaultAttack());
    SetCastReady(DefaultCastReady());
    SetYell(DefaultYell());
    SetJump(DefaultJump());
    SetCombat(DefaultCombat());
    SetEventTrigger(DefaultEventTrigger());
    SetCast(DefaultCast());
    SetPass(DefaultPass());
    SetCharCycle(DefaultCharCycle());
    SetQuest(DefaultQuest());
    SetQuickReference(DefaultQuickReference());
    SetRest(DefaultRest());
    SetTimeCalendar(DefaultTimeCalendar());
    SetAutoNotes(DefaultAutoNotes());
    SetMapBook(DefaultMapBook());
    SetLookUp(DefaultLookUp());
    SetLookDown(DefaultLookDown());
    SetCenterView(DefaultCenterView());
    SetZoomIn(DefaultZoomIn());
    SetZoomOut(DefaultZoomOut());
    SetFlyUp(DefaultFlyUp());
    SetFlyDown(DefaultFlyDown());
    SetLand(DefaultLand());
    SetAlwaysRun(DefaultAlwaysRun());
    SetStepLeft(DefaultStepLeft());
    SetStepRight(DefaultStepRight());
}

void GameConfig::Keybindings::Load() {
    SetForward(LoadOptionString(section_name, "forward", DefaultForward()));
    SetBackward(LoadOptionString(section_name, "backward", DefaultBackward()));
    SetLeft(LoadOptionString(section_name, "left", DefaultLeft()));
    SetRight(LoadOptionString(section_name, "right", DefaultRight()));
    SetAttack(LoadOptionString(section_name, "attack", DefaultAttack()));
    SetCastReady(LoadOptionString(section_name, "cast_ready", DefaultCastReady()));
    SetYell(LoadOptionString(section_name, "yell", DefaultYell()));
    SetJump(LoadOptionString(section_name, "jump", DefaultJump()));
    SetCombat(LoadOptionString(section_name, "combat", DefaultCombat()));
    SetEventTrigger(LoadOptionString(section_name, "event_trigger", DefaultEventTrigger()));
    SetCast(LoadOptionString(section_name, "cast", DefaultCast()));
    SetPass(LoadOptionString(section_name, "pass", DefaultPass()));
    SetCharCycle(LoadOptionString(section_name, "char_cycle", DefaultCharCycle()));
    SetQuest(LoadOptionString(section_name, "quest", DefaultQuest()));
    SetQuickReference(LoadOptionString(section_name, "quick_reference", DefaultQuickReference()));
    SetRest(LoadOptionString(section_name, "rest", DefaultRest()));
    SetTimeCalendar(LoadOptionString(section_name, "time_calendar", DefaultTimeCalendar()));
    SetAutoNotes(LoadOptionString(section_name, "auto_notes", DefaultAutoNotes()));
    SetMapBook(LoadOptionString(section_name, "map_book", DefaultMapBook()));
    SetLookUp(LoadOptionString(section_name, "look_up", DefaultLookUp()));
    SetLookDown(LoadOptionString(section_name, "look_down", DefaultLookDown()));
    SetCenterView(LoadOptionString(section_name, "center_view", DefaultCenterView()));
    SetZoomIn(LoadOptionString(section_name, "zoom_in", DefaultZoomIn()));
    SetZoomOut(LoadOptionString(section_name, "zoom_out", DefaultZoomOut()));
    SetFlyUp(LoadOptionString(section_name, "fly_up", DefaultFlyUp()));
    SetFlyDown(LoadOptionString(section_name, "fly_down", DefaultFlyDown()));
    SetLand(LoadOptionString(section_name, "land", DefaultLand()));
    SetAlwaysRun(LoadOptionString(section_name, "always_run", DefaultAlwaysRun()));
    SetStepLeft(LoadOptionString(section_name, "step_left", DefaultStepLeft()));
    SetStepRight(LoadOptionString(section_name, "step_right", DefaultStepRight()));
}

void GameConfig::Keybindings::Save() {
    SaveOption(section_name, "forward", GetForward());
    SaveOption(section_name, "backward", GetBackward());
    SaveOption(section_name, "left", GetLeft());
    SaveOption(section_name, "right", GetRight());
    SaveOption(section_name, "attack", GetAttack());
    SaveOption(section_name, "cast_ready", GetCastReady());
    SaveOption(section_name, "yell", GetYell());
    SaveOption(section_name, "jump", GetJump());
    SaveOption(section_name, "combat", GetCombat());
    SaveOption(section_name, "event_trigger", GetEventTrigger());
    SaveOption(section_name, "cast", GetCast());
    SaveOption(section_name, "pass", GetPass());
    SaveOption(section_name, "char_cycle", GetCharCycle());
    SaveOption(section_name, "quest", GetQuest());
    SaveOption(section_name, "quick_reference", GetQuickReference());
    SaveOption(section_name, "rest", GetRest());
    SaveOption(section_name, "time_calendar", GetTimeCalendar());
    SaveOption(section_name, "auto_notes", GetAutoNotes());
    SaveOption(section_name, "map_book", GetMapBook());
    SaveOption(section_name, "look_up", GetLookUp());
    SaveOption(section_name, "look_down", GetLookDown());
    SaveOption(section_name, "center_view", GetCenterView());
    SaveOption(section_name, "zoom_in", GetZoomIn());
    SaveOption(section_name, "zoom_out", GetZoomOut());
    SaveOption(section_name, "fly_up", GetFlyUp());
    SaveOption(section_name, "fly_down", GetFlyDown());
    SaveOption(section_name, "land", GetLand());
    SaveOption(section_name, "always_run", GetAlwaysRun());
    SaveOption(section_name, "step_left", GetStepLeft());
    SaveOption(section_name, "step_right", GetStepRight());
}

void GameConfig::Settings::Default() {
    SetAlwaysRun(DefaultAlwaysRun());
    SetFlipOnExit(DefaultFlipOnExit());
    SetShowHits(DefaultShowHits());
    SetMusicLevel(DefaultMusicLevel());
    SetSoundLevel(DefaultSoundLevel());
    SetVoiceLevel(DefaultVoiceLevel());
    SetTurnSpeed(DefaultTurnSpeed());
    SetVerticalTurnSpeed(DefaultVerticalSpeed());
    SetWalkSound(DefaultWalkSound());
}

void GameConfig::Settings::Load() {
    SetAlwaysRun(LoadOptionBool(section_name, "always_run", DefaultAlwaysRun()));
    SetFlipOnExit(LoadOptionBool(section_name, "flip_on_exit", DefaultFlipOnExit()));
    SetShowHits(LoadOptionBool(section_name, "show_hits", DefaultShowHits()));
    SetMusicLevel(LoadOptionInteger(section_name, "music_level", DefaultMusicLevel()));
    SetSoundLevel(LoadOptionInteger(section_name, "sound_level", DefaultSoundLevel()));
    SetVoiceLevel(LoadOptionInteger(section_name, "voice_level", DefaultVoiceLevel()));
    SetTurnSpeed(LoadOptionInteger(section_name, "turn_speed", DefaultTurnSpeed()));
    SetVerticalTurnSpeed(LoadOptionInteger(section_name, "vertical_turn_speed", DefaultVerticalSpeed()));
    SetWalkSound(LoadOptionBool(section_name, "walk_sound", DefaultWalkSound()));
}

void GameConfig::Settings::Save() {
    SaveOption(section_name, "always_run", GetAlwaysRun());
    SaveOption(section_name, "flip_on_exit", GetFlipOnExit());
    SaveOption(section_name, "show_hits", GetShowHits());
    SaveOption(section_name, "music_level", GetMusicLevel());
    SaveOption(section_name, "sound_level", GetSoundLevel());
    SaveOption(section_name, "voice_level", GetVoiceLevel());
    SaveOption(section_name, "turn_speed", GetTurnSpeed());
    SaveOption(section_name, "vertical_turn_speed", GetVerticalTurnSpeed());
    SaveOption(section_name, "walk_sound", GetWalkSound());
}

void GameConfig::Window::Default() {
    SetBorderless(DefaultBorderless());
    SetDisplay(DefaultDisplay());
    SetFullscreen(DefaultFullscreen());
    SetMouseGrab(DefaultMouseGrab());
    SetHeight(DefaultHeight());
    SetWidth(DefaultWidth());
    SetPositionX(DefaultPositionX());
    SetPositionY(DefaultPositionY());
    SetTitle(DefaultTitle());
}

void GameConfig::Window::Load() {
    SetBorderless(LoadOptionBool(section_name, "borderless", DefaultBorderless()));
    SetFullscreen(LoadOptionBool(section_name, "fullscreen", DefaultFullscreen()));
    SetDisplay(LoadOptionInteger(section_name, "display", DefaultDisplay()));
    SetPositionX(LoadOptionInteger(section_name, "position_x", DefaultPositionX()));
    SetPositionY(LoadOptionInteger(section_name, "position_y", DefaultPositionY()));
    SetWidth(LoadOptionInteger(section_name, "width", DefaultWidth()));
    SetHeight(LoadOptionInteger(section_name, "height", DefaultHeight()));
    SetMouseGrab(LoadOptionBool(section_name, "mouse_grab", DefaultMouseGrab()));
    SetTitle(LoadOptionString(section_name, "title", DefaultTitle()));
}

void GameConfig::Window::Save() {
    SaveOption(section_name, "borderless", GetBorderless());
    SaveOption(section_name, "fullscreen", GetFullscreen());
    SaveOption(section_name, "display", GetDisplay());
    SaveOption(section_name, "position_x", GetPositionX());
    SaveOption(section_name, "position_y", GetPositionY());
    SaveOption(section_name, "width", GetWidth());
    SaveOption(section_name, "height", GetHeight());
    SaveOption(section_name, "mouse_grab", GetMouseGrab());
    SaveOption(section_name, "title", GetTitle());
}

