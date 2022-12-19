#include "GameConfig.h"

#define MINI_CASE_SENSITIVE
#include <mini/ini.h>

#include <algorithm>
#include <string>

#include "Utility/DataPath.h"

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

GameConfig::~GameConfig() {
    ini.clear();
}
