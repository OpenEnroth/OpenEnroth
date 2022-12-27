#include "Config.h"

#define MINI_CASE_SENSITIVE
#include <mini/ini.h>

#include <stdexcept>
#include <filesystem>
#include <string>

void Config::Load(const std::string &path) {
    if (!std::filesystem::exists(path))
        throw std::runtime_error("Config file doesn't exist"); // TODO(captainurist): exception formatting

    mINI::INIFile file(path);
    mINI::INIStructure ini;
    if (!file.read(ini))
        throw std::runtime_error("Couldn't read config file"); // TODO(captainurist): exception formatting

    RunAll(loadCallbacks, &ini);
}

void Config::Save(const std::string &path) {
    mINI::INIFile file(path);
    mINI::INIStructure ini;
    RunAll(saveCallbacks, &ini);

    if (!file.write(ini, true))
        throw std::runtime_error("Couldn't save config file"); // TODO(captainurist): exception formatting
}

void Config::Reset() {
    RunAll(resetCallbacks, nullptr);
}

void Config::RunAll(std::vector<Callback> &callbacks, mINI::INIStructure *ini) {
    for(const Callback &callback : callbacks)
        callback(ini);
}

void Config::SaveOption(const std::string& value, mINI::INIStructure *ini, const std::string &sectionName, const std::string &valueName) {
    (*ini)[sectionName][valueName] = value;
}

void Config::LoadOption(std::string* value, mINI::INIStructure *ini, const std::string &sectionName, const std::string &valueName) {
    if (!ini->has(sectionName))
        return;

    const auto& section = (*ini)[sectionName];
    if (!section.has(valueName))
        return;

    *value = section.get(valueName);
}
