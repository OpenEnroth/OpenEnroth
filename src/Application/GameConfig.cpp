#include "GameConfig.h"

#include <algorithm>
#include <cctype>
#include <string>

#include "Utility/DataPath.h"

#define MINI_CASE_SENSITIVE
#include "thirdparty/mini/src/mini/ini.h"

using Application::GameConfig;

mINI::INIStructure ini;

void GameConfig::LoadOption(std::string section, GameConfig::ConfigValue<bool> *val) {
    bool r = true;
    std::string v = ini[section].get(val->Name());

    if (v.empty()) {
        val->Reset();
        return;
    }

    if (v == "false" || v == "0")
        r = false;

    val->Set(r);
}

void GameConfig::LoadOption(std::string section, GameConfig::ConfigValue<float> *val) {
    std::string v = ini[section].get(val->Name());

    if (v.empty()) {
        val->Reset();
        return;
    }

    val->Set(std::stof(v));
}

void GameConfig::LoadOption(std::string section, GameConfig::ConfigValue<int> *val) {
    std::string v = ini[section].get(val->Name());

    if (v.empty()) {
        val->Reset();
        return;
    }

    val->Set(std::stoi(v));
}

void GameConfig::LoadOption(std::string section, GameConfig::ConfigValue<std::string> *val) {
    std::string v = ini[section].get(val->Name());

    if (v.empty()) {
        val->Reset();
        return;
    }

    val->Set(v);
}

void GameConfig::SaveOption(std::string section, GameConfig::ConfigValue<bool> *val) {
    std::string v = "false";
    if (val->Get())
        v = "true";

    ini[section].set(val->Name(), v);
}

void GameConfig::SaveOption(std::string section, GameConfig::ConfigValue<int> *val) {
    ini[section].set(val->Name(), std::to_string(val->Get()));
}

void GameConfig::SaveOption(std::string section, GameConfig::ConfigValue<float> *val) {
    ini[section].set(val->Name(), std::to_string(val->Get()));
}

void GameConfig::SaveOption(std::string section, GameConfig::ConfigValue<std::string> *val) {
    ini[section].set(val->Name(), val->Get());
}

void GameConfig::LoadConfiguration() {
    std::string path = MakeDataPath(config_file);
    mINI::INIFile file(path);

    if (file.read(ini)) {
        LoadSections();
        logger->Info("Configuration file '%s' loaded!", path.c_str());
    } else {
        ResetSections();
        logger->Warning("Cound not read configuration file '%s'! Loaded default configuration instead!", path.c_str());
    }
}

void GameConfig::SaveConfiguration() {
    mINI::INIFile file(MakeDataPath(config_file));

    SaveSections();

    file.write(ini, true);
}

void GameConfig::Startup() {
    LoadConfiguration();

    std::shared_ptr<std::string> value;
    if (command_line->TryFindKey("-nointro")) {
        debug.NoIntro.Set(true);
    }
    if (command_line->TryFindKey("-nologo")) {
        debug.NoLogo.Set(true);
    }
    if (command_line->TryFindKey("-nosound")) {
        debug.NoSound.Set(true);
    }
    if (command_line->TryFindKey("-novideo")) {
        debug.NoVideo.Set(true);
    }
    if (command_line->TryFindKey("-nomarg")) {
        debug.NoMargareth.Set(true);
    }
    if (command_line->TryFindKey("-verbose")) {
        debug.VerboseLogging.Set(true);
    }
    if (command_line->TryGetValue("render", &value)) {
        graphics.Renderer.Set(*value);
    }
    if (command_line->TryFindKey("-nowalksound")) {
        settings.WalkSound.Set(false);
    }
    if (command_line->TryFindKey("-nograb")) {
        window.MouseGrab.Set(false);
    }
    if (command_line->TryGetValue("display", &value)) {
        window.Display.Set(std::stoi(*value));
    }
    if (command_line->TryGetValue("window_width", &value)) {
        window.Width.Set(std::stoi(*value));
    }
    if (command_line->TryGetValue("window_height", &value)) {
        window.Height.Set(std::stoi(*value));
    }
    if (command_line->TryGetValue("window_x", &value)) {
        window.PositionX.Set(std::stoi(*value));
    }
    if (command_line->TryGetValue("window_y", &value)) {
        window.PositionY.Set(std::stoi(*value));
    }
    if (command_line->TryGetValue("fullscreen", &value)) {
        window.Fullscreen.Set(std::stoi(*value));
    }
    if (command_line->TryGetValue("borderless", &value)) {
        window.Borderless.Set(std::stoi(*value));
    }
    // minimal debug, w/o full magic etc.
    if (command_line->TryFindKey("-debug")) {
        debug.ShowFPS.Set(true);
        debug.ShowPickedFace.Set(true);
        debug.TownPortal.Set(true);
        debug.InfiniteFood.Set(true);
        debug.InfiniteGold.Set(true);
    }
}

GameConfig::~GameConfig() {
    ini.clear();
}
